#include "pchdef.h"
#include "Database.h"
#include "Config.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;

namespace PktParser::Db
{

    Database::Database()
        :_poolHead{ nullptr }, _cluster{ nullptr }, _session{ nullptr }, _preparedInsert{ nullptr }
    {
        _cluster = cass_cluster_new();

        std::string cassandraHost = Config::GetCassandraHost();
        cass_cluster_set_contact_points(_cluster, cassandraHost.c_str());

        cass_cluster_set_queue_size_io(_cluster, 32768);
        cass_cluster_set_core_connections_per_host(_cluster, 4);
        cass_cluster_set_num_threads_io(_cluster, 8);
        cass_cluster_set_request_timeout(_cluster, 60000);
        cass_cluster_set_connect_timeout(_cluster, 15000);

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

        LOG("Database shutdown complete: {} inserted, {} failed", _totalInserted.load(), _totalFailed.load());
    }

    Database::InsertData *Database::AcquireInsertData()
    {
        InsertData* head = _poolHead.load(std::memory_order_acquire);

        while (head)
        {
            InsertData* next = head->next;

            if (_poolHead.compare_exchange_weak(head, next, std::memory_order_release, std::memory_order_acquire))
                return head;
        }

        return new InsertData();
    }

    void Database::ReleaseInsertData(InsertData *data)
    {
        InsertData* head = _poolHead.load(std::memory_order_acquire);

        do
        {
            data->next = head;
        } while (!_poolHead.compare_exchange_weak(head, data, std::memory_order_release, std::memory_order_acquire));
    }

    void Database::PrepareStmts()
    {
        char const* insertQuery =
            "INSERT INTO wow_packets.packets (packet_number, direction, packet_name, packet_len, opcode, timestamp, build, pkt_json) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

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

    void Database::StorePacket(json&& pkt)
    {
        while (_pendingCount.load(std::memory_order_relaxed) > 50000)
            std::this_thread::sleep_for(std::chrono::microseconds(100));

        _pendingCount.fetch_add(1, std::memory_order_relaxed);

        InsertData* data = AcquireInsertData();

        auto const& header = pkt["Header"];
        data->packetNumber = pkt["Number"].get<int>();
        data->direction = header["Direction"].get<std::string>();
        data->packetName = header["PacketName"].get<std::string>();
        data->packetLen = header["Length"].get<int>();
        data->opcode = header["Opcode"].get<std::string>();
        data->timestamp = header["Timestamp"].get<std::string>();
        data->build = header["Build"].get<int>();
        data->pktJson = std::move(pkt).dump();
        data->db = this;

        CassStatement* stmt = cass_prepared_bind(_preparedInsert);
        cass_statement_bind_int32(stmt, 0, data->packetNumber);
        cass_statement_bind_string(stmt, 1, data->direction.c_str());
        cass_statement_bind_string(stmt, 2, data->packetName.c_str());
        cass_statement_bind_int32(stmt, 3, data->packetLen);
        cass_statement_bind_string(stmt, 4, data->opcode.c_str());
        cass_statement_bind_string(stmt, 5, data->timestamp.c_str());
        cass_statement_bind_int32(stmt, 6, data->build);
        cass_statement_bind_string(stmt, 7, data->pktJson.c_str());

        CassFuture* future = cass_session_execute(_session, stmt);
        cass_future_set_callback(future, InsertCallback, data);

        cass_future_free(future);
        cass_statement_free(stmt);
    }

    void Database::InsertCallback(CassFuture* future, void* data)
    {
        InsertData* insertData = static_cast<InsertData*>(data);
        Database* db = insertData->db;
        CassError rc = cass_future_error_code(future);

        if (rc == CASS_OK)
        {
            db->_totalInserted.fetch_add(1, std::memory_order_relaxed);
            db->_pendingCount.fetch_sub(1, std::memory_order_relaxed);
            db->ReleaseInsertData(insertData);
            return;
        }

        bool isTimeout = (rc == CASS_ERROR_LIB_REQUEST_TIMED_OUT || rc == CASS_ERROR_SERVER_WRITE_TIMEOUT);
        bool canRetry = insertData->retryCount < 3;
        if (isTimeout && canRetry)
        {
            insertData->retryCount++;
            LOG("Retrying packet {} (attempt {}/3) - write timeout", insertData->packetNumber, insertData->retryCount);

            std::this_thread::sleep_for(std::chrono::milliseconds(500 * insertData->retryCount));

            CassStatement* stmt = cass_prepared_bind(db->_preparedInsert);
            cass_statement_bind_int32(stmt, 0, insertData->packetNumber);
            cass_statement_bind_string(stmt, 1, insertData->direction.c_str());
            cass_statement_bind_string(stmt, 2, insertData->packetName.c_str());
            cass_statement_bind_int32(stmt, 3, insertData->packetLen);
            cass_statement_bind_string(stmt, 4, insertData->opcode.c_str());
            cass_statement_bind_string(stmt, 5, insertData->timestamp.c_str());
            cass_statement_bind_int32(stmt, 6, insertData->build);
            cass_statement_bind_string(stmt, 7, insertData->pktJson.c_str());

            // retry
            CassFuture* retryFuture = cass_session_execute(db->_session, stmt);
            cass_future_set_callback(retryFuture, InsertCallback, insertData);
            
            cass_future_free(retryFuture);
            cass_statement_free(stmt);
            return;
        }

        // total failure
        db->_totalFailed.fetch_add(1, std::memory_order_relaxed);
        db->_pendingCount.fetch_sub(1, std::memory_order_relaxed);
        db->ReleaseInsertData(insertData);

        const char* msg;
        size_t msgLen;
        cass_future_error_message(future, &msg, &msgLen);
        LOG("INSERT PERMANENTLY FAILED [Packet {}] after {} attempts: {} - {}",
            insertData->packetNumber, insertData->retryCount + 1, cass_error_desc(rc), std::string(msg, msgLen));
    }

    void Database::Flush()
    {
        if (_pendingCount.load() > 0)
            LOG("Waiting for {} pending inserts...", _pendingCount.load());

        while (_pendingCount.load() > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

        LOG("FLUSH Complete: {} inserted, {} failed", _totalInserted.load(), _totalFailed.load());
    }
}
