#include "pchdef.h"
#include "Database.h"
#include "Config.h"
#include "Misc/Utilities.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;

namespace PktParser::Db
{

    Database::Database()
        :_poolHead{ nullptr }, _cluster{ nullptr }, _session{ nullptr }, _preparedInsert{ nullptr }
    {
        _cluster = cass_cluster_new();
        
        cass_cluster_set_timestamp_gen(_cluster, cass_timestamp_gen_server_side_new());

        std::string cassandraHost = Config::GetCassandraHost();
        cass_cluster_set_contact_points(_cluster, cassandraHost.c_str());

        cass_cluster_set_queue_size_io(_cluster, MAX_PENDING);
        cass_cluster_set_core_connections_per_host(_cluster, 2);
        cass_cluster_set_num_threads_io(_cluster, 4);
        cass_cluster_set_request_timeout(_cluster, 30000);
        cass_cluster_set_connect_timeout(_cluster, 15000);

        CassRetryPolicy* retryPolicy = cass_retry_policy_default_new();
        cass_cluster_set_retry_policy(_cluster, retryPolicy);
        cass_retry_policy_free(retryPolicy);

        _session = cass_session_new();
        CassFuture* connectFuture = cass_session_connect_keyspace(_session, _cluster, "wow_packets");

        if (cass_future_error_code(connectFuture) != CASS_OK)
        {
            const char* msg;
            size_t msgLen;
            cass_future_error_message(connectFuture, &msg, &msgLen);
            cass_future_free(connectFuture);
            throw ParseException{ std::string("Cannot connect to Cassandra: ") + std::string(msg, msgLen) };
        }

        cass_future_free(connectFuture);
        LOG("The bluetooth device has connected successfully");
        
        PrepareStmts();

        _callbackContext.totalInserted = &_totalInserted;
        _callbackContext.totalFailed = &_totalFailed;
        _callbackContext.pendingCount = &_pendingCount;
        _callbackContext.poolHead = &_poolHead;
        _callbackContext.preparedStmt = _preparedInsert;
        _callbackContext.session = _session;
    }

    Database::~Database()
    {
        LOG("Shutting down db...");

        Flush();

        if (_preparedInsert)
            cass_prepared_free(_preparedInsert);

        CassFuture* closeFuture = cass_session_close(_session);
        cass_future_wait(closeFuture);
        cass_future_free(closeFuture);

        cass_session_free(_session);
        cass_cluster_free(_cluster);

        InsertData* head = _poolHead.load();
        while (head)
        {
            InsertData* next = head->next;
            delete head;
            head = next;
        }

        LOG("Database shutdown complete: {} inserted, {} failed, {:.2f} MB written", _totalInserted.load(), _totalFailed.load(),
            _totalBytes.load() / (1024.0 * 1024.0));
    }

    InsertData *Database::AcquireInsertData()
    {
        InsertData* head = _poolHead.load(std::memory_order_acquire);

        while (head)
        {
            InsertData* next = head->next;

            if (_poolHead.compare_exchange_weak(head, next, std::memory_order_release, std::memory_order_acquire))
            {
                head->retryCount = 0;
                return head;
            }
        }

        return new InsertData();
    }

    void Database::PrepareStmts()
    {
        char const* insertQuery =
            "INSERT INTO wow_packets.packets "
            "(build, file_id, bucket, packet_number, source_file, direction, packet_name, packet_len, opcode, timestamp, pkt_json) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

        CassFuture* prepareFuture = cass_session_prepare(_session, insertQuery);
        cass_future_wait(prepareFuture);
        
        if (cass_future_error_code(prepareFuture) != CASS_OK)
        {
            char const* msg;
            size_t msgLen;
            cass_future_error_message(prepareFuture, &msg, &msgLen);
            cass_future_free(prepareFuture);
            throw ParseException{ std::string("Cannot prepare statement :( : ") + std::string(msg, msgLen) };
        }

        _preparedInsert = const_cast<CassPrepared*>(cass_future_get_prepared(prepareFuture));
        cass_future_free(prepareFuture);
    }

	void Database::StorePacket(Reader::PktHeader const& header, char const* opcodeName, uint32 build, uint32 pktNumber, json&& packetData, std::string const& srcFile, CassUuid const& fileId)
    {
        while (_pendingCount.load(std::memory_order_relaxed) >= MAX_PENDING)
            std::this_thread::sleep_for(std::chrono::microseconds(100));

        _pendingCount.fetch_add(1, std::memory_order_relaxed);

        InsertData* data = AcquireInsertData();

        try
        {
            data->build = build;
            data->fileId = fileId;
            data->bucket = pktNumber / 10000;
            data->packetNumber = pktNumber;
            data->sourceFile = srcFile;
            data->direction = Misc::DirectionToString(header.direction);
            data->packetName = opcodeName;
            data->packetLen = header.packetLength - 4;
            data->opcode = header.opcode;
            data->timestamp = static_cast<int64>(header.timestamp * 1000);
            data->pktJson = std::move(packetData).dump();
            data->context = &_callbackContext;

            _totalBytes.fetch_add(data->pktJson.size(), std::memory_order_relaxed);
        }
        catch (std::exception const& e)
        {
            LOG("Failed to prepare packet {}: {}", data->packetNumber, e.what());
            _callbackContext.ReleaseToPool(data);
            _pendingCount.fetch_sub(1, std::memory_order_relaxed);
            return;
        }

        CassStatement* stmt = cass_prepared_bind(_preparedInsert);
        BindInsertStatement(stmt, data);

        CassFuture* future = cass_session_execute(_session, stmt);
        cass_future_set_callback(future, InsertCallback, data);

        cass_statement_free(stmt);
    }

    void Database::RetryInsert(InsertData *data)
    {
        CallbackContext* ctx = data->context;

        CassStatement* stmt = cass_prepared_bind(ctx->preparedStmt);
        BindInsertStatement(stmt, data);

        CassFuture* retryFuture = cass_session_execute(ctx->session, stmt);
        cass_future_set_callback(retryFuture, InsertCallback, data);
        
        cass_statement_free(stmt);
    }

    void Database::InsertCallback(CassFuture* future, void* data)
    {
        InsertData* insertData = static_cast<InsertData*>(data);
        CallbackContext* ctx = insertData->context;
        CassError rc = cass_future_error_code(future);

        cass_future_free(future);

        if (rc == CASS_OK)
        {
            ctx->totalInserted->fetch_add(1, std::memory_order_relaxed);
            ctx->pendingCount->fetch_sub(1, std::memory_order_relaxed);
            ctx->ReleaseToPool(insertData);
            return;
        }

        bool isTimeout = (rc == CASS_ERROR_LIB_REQUEST_TIMED_OUT || rc == CASS_ERROR_SERVER_WRITE_TIMEOUT);
        if (isTimeout && insertData->retryCount < 3)
        {
            insertData->retryCount++;
            LOG("Retrying packet {} (attempt {}/3) - write timeout", insertData->packetNumber, insertData->retryCount);
            
            RetryInsert(insertData);
            return;
        }

        int failedPacketNumber = insertData->packetNumber;
        int attemptCount = insertData->retryCount + 1;
        std::string errDesc = cass_error_desc(rc);

        // total failure
        ctx->totalFailed->fetch_add(1, std::memory_order_relaxed);
        ctx->pendingCount->fetch_sub(1, std::memory_order_relaxed);
        ctx->ReleaseToPool(insertData);

        LOG("INSERT PERMANENTLY FAILED [Packet {}] after {} attempts: {}", failedPacketNumber, attemptCount, errDesc);
    }

    void Database::BindInsertStatement(CassStatement* stmt, InsertData const* data)
    {
        cass_statement_bind_int32(stmt, 0, data->build);
        cass_statement_bind_uuid(stmt, 1, data->fileId);
        cass_statement_bind_int32(stmt, 2, data->bucket);
        cass_statement_bind_int32(stmt, 3, data->packetNumber);
        cass_statement_bind_string(stmt, 4, data->sourceFile.c_str());
        cass_statement_bind_string(stmt, 5, data->direction.c_str());
        cass_statement_bind_string(stmt, 6, data->packetName.c_str());
        cass_statement_bind_int32(stmt, 7, data->packetLen);
        cass_statement_bind_int32(stmt, 8, data->opcode);
        cass_statement_bind_int64(stmt, 9, data->timestamp);
        cass_statement_bind_bytes(stmt, 10, reinterpret_cast<const cass_byte_t*>(data->pktJson.data()), data->pktJson.size());
    }

    void Database::Flush()
    {
        if (_pendingCount.load() > 0)
            LOG("Waiting for {} pending inserts...", _pendingCount.load());

        while (_pendingCount.load() > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

        LOG("FLUSH Complete: {} inserted, {} failed", _totalInserted.load(), _totalFailed.load());
    }

    CassUuid Database::GenerateFileId(uint32 startTime, size_t fileSize)
    {
        std::hash<uint64> hasher;

        uint64 combined1 = (static_cast<uint64>(startTime) << 32) | static_cast<uint64>(fileSize & 0xFFFFFFFF);
        uint64 combined2 = (static_cast<uint64>(fileSize) << 32) | static_cast<uint64>(startTime);
        
        CassUuid uuid;
        uuid.time_and_version = hasher(combined1);
        uuid.clock_seq_and_node = hasher(combined2);

        uuid.time_and_version = (uuid.time_and_version & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        uuid.clock_seq_and_node = (uuid.clock_seq_and_node & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        return uuid;
    }

    void CallbackContext::ReleaseToPool(InsertData *data)
    {
        InsertData* head = poolHead->load(std::memory_order_acquire);

        do
        {
            data->next = head;
        } while (!poolHead->compare_exchange_weak(head, data, std::memory_order_release, std::memory_order_acquire));
    }
}
