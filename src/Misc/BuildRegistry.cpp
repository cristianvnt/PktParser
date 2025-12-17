#include "BuildRegistry.h"
#include "Logger.h"
#include "Database/Database.h"

#include <cassandra.h>

using namespace PktParser::Db;

namespace PktParser::Misc
{
    std::unordered_map<uint32, BuildMappings> BuildRegistry::_mappings;
    std::mutex BuildRegistry::_mutex;
    bool BuildRegistry::_initialized = false;

    void BuildRegistry::LoadFromDb(Database& db)
    {
        _mappings.clear();

        char const* query = "SELECT patch, build, deploy_timestamp, parser_version FROM wow_packets.build_mappings";
                            
        CassStatement* stmt = cass_statement_new(query, 0);
        CassFuture* future = cass_session_execute(db.GetSession(), stmt);

        CassResult const* result = cass_future_get_result(future);
        if (!result)
        {
            LOG("WARN: Couldnt load build mappings from database");
            cass_future_free(future);
            cass_statement_free(stmt);
            return;
        }

        CassIterator* rows = cass_iterator_from_result(result);
        while (cass_iterator_next(rows))
        {
            CassRow const* row = cass_iterator_get_row(rows);

            BuildMappings mapping;
            
            char const* patch;
            size_t patchLen;
            cass_value_get_string(cass_row_get_column(row, 0), &patch, &patchLen);
            mapping.Patch = std::string(patch, patchLen);

            cass_value_get_int32(cass_row_get_column(row, 1), (cass_int32_t*)&mapping.Build);

            cass_value_get_int64(cass_row_get_column(row, 2), &mapping.DeployTimestamp);

            char const* parserVer;
            size_t parserVerLen;
            cass_value_get_string(cass_row_get_column(row, 3), &parserVer, &parserVerLen);
            mapping.ParserVersion = std::string(parserVer, parserVerLen);

            _mappings[mapping.Build] = mapping;
        }

        cass_iterator_free(rows);
        cass_result_free(result);
        cass_future_free(future);
        cass_statement_free(stmt);
    }

    void BuildRegistry::Initialize(Database& db)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_initialized)
            return;

        LoadFromDb(db);
        _initialized = true;
        LOG("BuildRegistry initialized with {} mappings", _mappings.size());
    }

    BuildMappings const* BuildRegistry::GetMappings(uint32 build)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (auto it = _mappings.find(build); it != _mappings.end())
            return &it->second;
        return nullptr;
    }

    bool BuildRegistry::IsSupported(uint32 build)
    {
        return GetMappings(build) != nullptr;
    }

    void BuildRegistry::Reload(Database& db)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        LOG("Reloading build mappings from db...");
        LoadFromDb(db);
        LOG("Reloaded {} build mappings", _mappings.size());
    }
}