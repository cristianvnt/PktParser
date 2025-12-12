#include "Database.h"

#include "Misc/Define.h"
#include "Misc/Exceptions.h"

using namespace PktParser::Reader;

namespace PktParser::Db
{

    Database::Database()
        :_cluster{ nullptr }, _session{ nullptr }, _preparedInsert{ nullptr }
    {
        _batch.reserve(BATCH_SIZE);

        _cluster = cass_cluster_new();
        cass_cluster_set_contact_points(_cluster, "127.0.0.1");

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
        LOG("Your bluetooth device has connected successfuly!");
        
        CreateKeyspaceAndTable();
        PrepareStmts();
    }

    Database::~Database()
    {
        Flush();

        if (_preparedInsert)
            cass_prepared_free(_preparedInsert);

        CassFuture* closeFuture = cass_session_close(_session);
        //cass_future_wait(closeFuture);
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
                "opcode text,"
                "timestamp text,"
                "build int"
            ")";

        stmt = cass_statement_new(createTable, 0);
        future = cass_session_execute(_session, stmt);

        if (cass_future_error_code(future) != CASS_OK)
        {
            const char* msg;
            size_t msgLen;
            cass_future_error_message(future, &msg, &msgLen);
            throw ParseException{ "Could not create table: " + std::string(msg, msgLen) };
        }
        
        cass_future_free(future);
        cass_statement_free(stmt);

        LOG("READYYYYYYY");
    }

    void Database::PrepareStmts()
    {
        char const* insertQuery =
            "INSERT INTO wow_packets.packets (packet_number, direction, packet_name, opcode, timestamp, build) "
            "VALUES (?, ?, ?, ?, ?, ?)";

        CassFuture* prepareFuture = cass_session_prepare(_session, insertQuery);
        
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

    void Database::StorePacket(json const& pkt)
    {
        _batch.push_back(pkt);
        if (_batch.size() >= BATCH_SIZE)
            Flush();
    }

    void Database::Flush()
    {
        if (_batch.empty())
            return;

        const size_t MAX_BATCH_SIZE_BYTES = 45000;
        size_t index = 0;
            
        while (index < _batch.size())
        {
            CassBatch* batch = cass_batch_new(CASS_BATCH_TYPE_UNLOGGED);
            size_t batchSizeBytes = 0;

            size_t batchStartIndex = index;
            std::string pktJson;

            while (index < _batch.size())
            {
                const json& pkt = _batch[index];
                pktJson = pkt.dump();
                size_t pktSize = pktJson.size() + 100;

                if (batchSizeBytes + pktSize > MAX_BATCH_SIZE_BYTES)
                    break; // batch full

                CassStatement* stmt = cass_prepared_bind(_preparedInsert);
                cass_statement_bind_int32(stmt, 0, pkt["Number"].get<int>());
                cass_statement_bind_string(stmt, 1, pkt["Header"]["Direction"].get<std::string>().c_str());
                cass_statement_bind_string(stmt, 2, pkt["Header"]["PacketName"].get<std::string>().c_str());
                cass_statement_bind_string(stmt, 3, pkt["Header"]["Opcode"].get<std::string>().c_str());
                cass_statement_bind_string(stmt, 4, pkt["Header"]["Timestamp"].get<std::string>().c_str());
                cass_statement_bind_int32(stmt, 5, pkt["Header"]["Build"].get<int>());
                //cass_statement_bind_string(stmt, 6, pkt.dump().c_str());

                cass_batch_add_statement(batch, stmt);
                cass_statement_free(stmt);

                batchSizeBytes += pktSize;
                index++;
            }

            //LOG("Inserting batch of {} packets...", index - batchStartIndex);
        
            CassFuture* batchFuture = cass_session_execute_batch(_session, batch);
            cass_future_free(batchFuture);
            cass_batch_free(batch);
        }

         LOG("Flushed {} packets safely", _batch.size());
        _batch.clear();
    }
}
