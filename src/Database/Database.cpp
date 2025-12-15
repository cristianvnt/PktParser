#include "Database.h"

#include "Misc/Define.h"
#include "Misc/Exceptions.h"
#include "Misc/Logger.h"

using namespace PktParser::Reader;

namespace PktParser::Db
{

    Database::Database(size_t maxPending /*= 1000*/, size_t batchFlushSize /*= 100*/)
        :_cluster{ nullptr }, _session{ nullptr }, _preparedInsert{ nullptr },
        _maxPending{ maxPending }, _batchFlushSize{ batchFlushSize }
    {
        _cluster = cass_cluster_new();
        cass_cluster_set_contact_points(_cluster, "127.0.0.1");

        cass_cluster_set_queue_size_io(_cluster, 32768);
        cass_cluster_set_core_connections_per_host(_cluster, 2);
        cass_cluster_set_num_threads_io(_cluster, 4);

        _session = cass_session_new();

        CassFuture* connectFuture = cass_session_connect(_session, _cluster);

        if (cass_future_error_code(connectFuture) != CASS_OK)
        {
            const char* msg;
            size_t msgLen;
            cass_future_error_message(connectFuture, &msg, &msgLen);
            cass_future_free(connectFuture);
            throw ParseException{ std::string("Cannot connect to Cassandra: ") + std::string(msg, msgLen) };
        }

        cass_future_free(connectFuture);
        
        CreateKeyspaceAndTable();
        PrepareStmts();

        LOG("The bluetooth device has connected successfully");
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
    }

    void Database::CreateKeyspaceAndTable()
    {
        char const* createKeyspace = "CREATE KEYSPACE IF NOT EXISTS wow_packets "
            "WITH replication = {'class': 'SimpleStrategy', 'replication_factor': 1}";

        CassStatement* stmt = cass_statement_new(createKeyspace, 0);
        CassFuture* future = cass_session_execute(_session, stmt);
        cass_future_wait(future);

        if (cass_future_error_code(future) != CASS_OK)
        {
            char const* msg;
            size_t msgLen;
            cass_future_error_message(future, &msg, &msgLen);
            throw ParseException{ "Could not create keyspace: " + std::string(msg, msgLen) };
        }

        cass_future_free(future);
        cass_statement_free(stmt);

        char const* createTable =
            "CREATE TABLE IF NOT EXISTS wow_packets.packets ("
                "packet_number int PRIMARY KEY,"
                "direction text,"
                "packet_name text,"
                "packet_len int,"
                "opcode text,"
                "timestamp text,"
                "build int,"
                //"pkt_bson blob"
                "pkt_json text"
            ")";

        stmt = cass_statement_new(createTable, 0);
        future = cass_session_execute(_session, stmt);
        cass_future_wait(future);

        if (cass_future_error_code(future) != CASS_OK)
        {
            const char* msg;
            size_t msgLen;
            cass_future_error_message(future, &msg, &msgLen);
            throw ParseException{ "Could not create table: " + std::string(msg, msgLen) };
        }
        
        cass_future_free(future);
        cass_statement_free(stmt);
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

    void Database::CheckOldestInserts(size_t count)
    {
        size_t toCheck = std::min(count, _pendingInserts.size());

        for (size_t i = 0; i < toCheck; ++i)
        {
            CassFuture* future = _pendingInserts.front();
            _pendingInserts.pop_front();

            cass_future_wait(future);

            if (cass_future_error_code(future) == CASS_OK)
                _totalInserted.fetch_add(1, std::memory_order_relaxed);
            else
            {
                _totalFailed.fetch_add(1, std::memory_order_relaxed);

                const char* msg;
                size_t msgLen;
                cass_future_error_message(future, &msg, &msgLen);
                LOG("Insert failed: {}", std::string(msg, msgLen));
            }

            cass_future_free(future);
        }
    }

    void Database::StorePacket(json const& pkt)
    {
        bool needsFlush = false;
        {
            std::lock_guard<std::mutex> lock(_pendingMutex);
            needsFlush = _pendingInserts.size() >= _maxPending;
        }

        if (needsFlush)
        {
            std::lock_guard<std::mutex> lock(_pendingMutex);
            if (_pendingInserts.size() >= _maxPending)
                CheckOldestInserts(_batchFlushSize);
        }

        CassStatement* stmt = cass_prepared_bind(_preparedInsert);

        cass_statement_bind_int32(stmt, 0, pkt["Number"].get<int>());
        cass_statement_bind_string(stmt, 1, pkt["Header"]["Direction"].get<std::string>().c_str());
        cass_statement_bind_string(stmt, 2, pkt["Header"]["PacketName"].get<std::string>().c_str());
        cass_statement_bind_int32(stmt, 3, pkt["Header"]["Length"].get<int>());
        cass_statement_bind_string(stmt, 4, pkt["Header"]["Opcode"].get<std::string>().c_str());
        cass_statement_bind_string(stmt, 5, pkt["Header"]["Timestamp"].get<std::string>().c_str());
        cass_statement_bind_int32(stmt, 6, pkt["Header"]["Build"].get<int>());
        cass_statement_bind_string(stmt, 7, pkt.dump(4).c_str());

        CassFuture* future = cass_session_execute(_session, stmt);

        {
            std::lock_guard<std::mutex> lock(_pendingMutex);
            _pendingInserts.push_back(future);
        }

        cass_statement_free(stmt);
    }

    void Database::Flush()
    {
        std::lock_guard<std::mutex> lock(_pendingMutex);

        if (_pendingInserts.empty())
        {
            LOG("No pending inserts to flush");
            return;
        }

        LOG("Flushing {} pending inserts...", _pendingInserts.size());
        CheckOldestInserts(_pendingInserts.size());

        LOG("FLUSH Complete: {} inserted, {} failed", _totalInserted.load(), _totalFailed.load());
    }
}
