#include "pchdef.h"
#include "Database.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;

namespace PktParser::Db
{

    Database::Database()
        :_cluster{ nullptr }, _session{ nullptr }, _preparedInsert{ nullptr }
    {
        _cluster = cass_cluster_new();
        cass_cluster_set_contact_points(_cluster, "127.0.0.1");

        cass_cluster_set_queue_size_io(_cluster, 32768);
        cass_cluster_set_core_connections_per_host(_cluster, 4);
        cass_cluster_set_num_threads_io(_cluster, 8);

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
        LOG("The bluetooth device has connected successfully");
        
        CreateKeyspaceAndTable();
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

        LOG("Database shutdown complete: {} inserted, {} failed", _totalInserted.load(), _totalFailed.load());
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
            "CREATE TABLE IF NOT EXISTS wow_packets.packets("
                "packet_number int PRIMARY KEY,"
                "direction text,"
                "packet_name text,"
                "packet_len int,"
                "opcode text,"
                "timestamp text,"
                "build int,"
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
            cass_future_free(future);
            cass_statement_free(stmt);
            throw ParseException{ "Could not create >packets< table: " + std::string(msg, msgLen) };
        }
        
        cass_future_free(future);
        cass_statement_free(stmt);

        const char* createBuildMappingsTable = 
            "CREATE TABLE IF NOT EXISTS wow_packets.build_mappings ("
                "patch text,"
                "build int,"
                "deploy_timestamp timestamp,"
                "parser_version text,"
                "PRIMARY KEY (patch, build)"
            ")";

        stmt = cass_statement_new(createBuildMappingsTable, 0);
        future = cass_session_execute(_session, stmt);
        cass_future_wait(future);

        if (cass_future_error_code(future) != CASS_OK)
        {
            const char* msg;
            size_t msgLen;
            cass_future_error_message(future, &msg, &msgLen);
            throw ParseException{ "Could not create >build_mappings< table: " + std::string(msg, msgLen) };
        }

        cass_future_free(future);
        cass_statement_free(stmt);

        LOG("Keyspace and tables ready");
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
        _pendingCount.fetch_add(1, std::memory_order_relaxed);

        CassStatement* stmt = cass_prepared_bind(_preparedInsert);

        auto const& header = pkt["Header"];
        int packetNumber = pkt["Number"].get<int>();
        std::string direction = header["Direction"].get<std::string>();
        std::string packetName = header["PacketName"].get<std::string>();
        int packetLen = header["Length"].get<int>();
        std::string opcode = header["Opcode"].get<std::string>();
        std::string timestamp = header["Timestamp"].get<std::string>();
        int build = header["Build"].get<int>();
        std::string pktJson = std::move(pkt).dump();

        cass_statement_bind_int32(stmt, 0, packetNumber);
        cass_statement_bind_string(stmt, 1, direction.c_str());
        cass_statement_bind_string(stmt, 2, packetName.c_str());
        cass_statement_bind_int32(stmt, 3, packetLen);
        cass_statement_bind_string(stmt, 4, opcode.c_str());
        cass_statement_bind_string(stmt, 5, timestamp.c_str());
        cass_statement_bind_int32(stmt, 6, build);
        cass_statement_bind_string(stmt, 7, pktJson.c_str());

        CassFuture* future = cass_session_execute(_session, stmt);

        cass_future_set_callback(future, InsertCallback, this);

        cass_future_free(future);
        cass_statement_free(stmt);
    }

    void Database::InsertCallback(CassFuture* future, void* data)
    {
        Database* db = static_cast<Database*>(data);

        if (cass_future_error_code(future) == CASS_OK)
            db->_totalInserted.fetch_add(1, std::memory_order_relaxed);
        else
            db->_totalFailed.fetch_add(1, std::memory_order_relaxed);
        
        db->_pendingCount.fetch_sub(1, std::memory_order_relaxed);
    }

    void Database::Flush()
    {
        if (_pendingCount.load() > 0)
            LOG("Waiting for {} pending inserts...", _pendingCount.load());

        while (_pendingCount.load() > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

        LOG("FLUSH Complete: {} inserted, {} failed", _totalInserted.load(), _totalFailed.load());
    }

    // extra
    void Database::InsertBuildMapping(BuildMappings const& mapping)
    {
        static char const* query =
        "INSERT INTO wow_packets.build_mappings (patch, build, deploy_timestamp, parser_version) "
        "VALUES (?, ?, ?, ?) IF NOT EXISTS";

        CassStatement* stmt = cass_statement_new(query, 4);

        cass_statement_bind_string(stmt, 0, mapping.Patch.c_str());
        cass_statement_bind_int32(stmt, 1, mapping.Build);
        cass_statement_bind_int64(stmt, 2, mapping.DeployTimestamp);
        cass_statement_bind_string(stmt, 3, mapping.ParserVersion.c_str());

        CassFuture* future = cass_session_execute(_session, stmt);
        cass_future_wait(future);

        if (cass_future_error_code(future) != CASS_OK)
        {
            const char* msg;
            size_t len;
            cass_future_error_message(future, &msg, &len);
            LOG("Failed to insert build mapping {}: {}", mapping.Build, std::string(msg, len));
        }

        cass_future_free(future);
        cass_statement_free(stmt);
    }
}
