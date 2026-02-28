#include "pchdef.h"
#include "Database.h"
#include "Config.h"
#include "Misc/Utilities.h"
#include <zstd.h>

using namespace PktParser::Reader;
using namespace PktParser::Misc;

namespace PktParser::Db
{

    Database::Database()
        : _cluster{ nullptr }, _session{ nullptr }, _preparedInsert{ nullptr }, _preparedMetadata{ nullptr }, _timestampGen{ nullptr }
    {
        _cluster = cass_cluster_new();
        
        _timestampGen = cass_timestamp_gen_server_side_new();
        cass_cluster_set_timestamp_gen(_cluster, _timestampGen);

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
        _callbackContext.poolMutex = &_poolMutex;
        _callbackContext.pool = &_pool;
        _callbackContext.preparedStmt = _preparedInsert;
        _callbackContext.session = _session;
    }

    Database::~Database()
    {
        LOG("Shutting down db...");

        Flush();

        if (_preparedInsert)
            cass_prepared_free(_preparedInsert);

        if (_preparedMetadata)
            cass_prepared_free(_preparedMetadata);
        
        CassFuture* closeFuture = cass_session_close(_session);
        cass_future_wait(closeFuture);
        cass_future_free(closeFuture);

        cass_session_free(_session);
        cass_cluster_free(_cluster);

        if (_timestampGen)
            cass_timestamp_gen_free(_timestampGen);

        for (InsertData* data : _pool)
            delete data;
        _pool.clear();

        if (_totalInserted.load() > 0 || _totalFailed.load() > 0)
        {
            double totalMB = _totalBytes.load() / (1024.0 * 1024.0);
            double compressedMB = _totalCompressedBytes.load() / (1024.0 * 1024.0);

            LOG("Database shutdown complete: {} inserted, {} failed, {:.2f} MB written",
                _totalInserted.load(), _totalFailed.load(), _totalBytes.load() / (1024.0 * 1024.0));
            LOG("Storage: {:.2f} MB raw -> {:.2f} MB compressed", totalMB, compressedMB);
        }
    }

    InsertData* Database::AcquireInsertData()
    {
        std::lock_guard<std::mutex> lock(_poolMutex);

        if (!_pool.empty())
        {
            InsertData* data = _pool.back();
            _pool.pop_back();
            data->retryCount = 0;
            return data;
        }

        return new InsertData();
    }

    void Database::PrepareStmts()
    {
        char const* insertQuery =
            "INSERT INTO wow_packets.packets "
            "(build, file_id, bucket, packet_number, direction, packet_len, opcode, timestamp, pkt_json) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

        CassFuture* prepareFuture = cass_session_prepare(_session, insertQuery);
        cass_future_wait(prepareFuture);
        
        if (cass_future_error_code(prepareFuture) != CASS_OK)
        {
            char const* msg;
            size_t msgLen;
            cass_future_error_message(prepareFuture, &msg, &msgLen);
            cass_future_free(prepareFuture);
            throw ParseException{ std::string("Cannot prepare insert: ") + std::string(msg, msgLen) };
        }

        _preparedInsert = const_cast<CassPrepared*>(cass_future_get_prepared(prepareFuture));
        cass_future_free(prepareFuture);

        char const* metadataQuery =
            "INSERT INTO wow_packets.file_metadata "
            "(file_id, source_file, build, start_time, packet_count) "
            "VALUES (?, ?, ?, ?, ?)";

        prepareFuture = cass_session_prepare(_session, metadataQuery);
        cass_future_wait(prepareFuture);
        
        if (cass_future_error_code(prepareFuture) != CASS_OK)
        {
            char const* msg;
            size_t msgLen;
            cass_future_error_message(prepareFuture, &msg, &msgLen);
            cass_future_free(prepareFuture);
            throw ParseException{ std::string("Cannot prepare metadata: ") + std::string(msg, msgLen) };
        }

        _preparedMetadata = const_cast<CassPrepared*>(cass_future_get_prepared(prepareFuture));
        cass_future_free(prepareFuture);
    }

    void Database::StoreFileMetadata(CassUuid const& fileId, std::string const& srcFile, uint32 build, int64 startTime, uint32 pktCount)
    {
        CassStatement* stmt = cass_prepared_bind(_preparedMetadata);

        cass_statement_bind_uuid(stmt, 0, fileId);
        cass_statement_bind_string(stmt, 1, srcFile.c_str());
        cass_statement_bind_int32(stmt, 2, build);
        cass_statement_bind_int64(stmt, 3, startTime);
        cass_statement_bind_int32(stmt, 4, pktCount);

        CassFuture* future = cass_session_execute(_session, stmt);
        cass_future_wait(future);

        if (cass_future_error_code(future) != CASS_OK)
        {
            char const* msg;
            size_t msgLen;
            cass_future_error_message(future, &msg, &msgLen);
            LOG("Failed to insert file metadata: {}", std::string(msg, msgLen));
        }

        cass_future_free(future);
        cass_statement_free(stmt);
    }

    void Database::StorePacket(Reader::PktHeader const& header, uint32 build, uint32 pktNumber, std::string&& jsonStr, CassUuid const& fileId)
    {
        while (_pendingCount.load(std::memory_order_relaxed) >= MAX_PENDING)
            std::this_thread::sleep_for(std::chrono::microseconds(100));

        _pendingCount.fetch_add(1, std::memory_order_relaxed);

        InsertData* data = AcquireInsertData();

        try
        {
            _totalBytes.fetch_add(jsonStr.size(), std::memory_order_relaxed);

            //data->compressedJson = Misc::CompressJson(jsonStr);
            _totalCompressedBytes.fetch_add(data->compressedJson.size(), std::memory_order_relaxed);

            data->build = build;
            data->fileId = fileId;
            data->bucket = pktNumber / 10000;
            data->packetNumber = pktNumber;
            data->direction = static_cast<uint8>(header.direction);
            data->packetLen = header.packetLength - 4;
            data->opcode = header.opcode;
            data->timestamp = static_cast<int64>(header.timestamp * 1000);
            data->context = &_callbackContext;
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

    void Database::RetryInsert(InsertData* data)
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
        cass_statement_bind_int8(stmt, 4, static_cast<uint8>(data->direction));
        cass_statement_bind_int32(stmt, 5, data->packetLen);
        cass_statement_bind_int32(stmt, 6, data->opcode);
        cass_statement_bind_int64(stmt, 7, data->timestamp);
        cass_statement_bind_bytes(stmt, 8, reinterpret_cast<const cass_byte_t*>(data->compressedJson.data()), data->compressedJson.size());
    }

    void Database::Flush()
    {
        if (_pendingCount.load() > 0)
        {
            LOG("Waiting for {} pending inserts...", _pendingCount.load());

            while (_pendingCount.load() > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));

            LOG("FLUSH Complete: {} inserted, {} failed", _totalInserted.load(), _totalFailed.load());
        }
    }

    void CallbackContext::ReleaseToPool(InsertData* data)
    {
        std::lock_guard<std::mutex> lock(*poolMutex);
        pool->push_back(data);
    }
}
