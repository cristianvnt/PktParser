#include "pchdef.h"
#include "BuildInfo.h"
#include "Config.h"

namespace PktParser::Db
{
    BuildInfo& BuildInfo::Instance()
    {
        static BuildInfo instance;
        return instance;
    }

    void BuildInfo::Initialize()
    {
        try
        {
            std::string connStr = Config::GetPostgresConnectionString();

            pqxx::connection conn(connStr);
            pqxx::work txn(conn);

            std::string query = "SELECT build_number, patch_version, parser_version "
                                "FROM builds "
                                "WHERE parser_version IS NOT NULL";
            
            pqxx::result res = txn.exec(query);
            
            _mappings.clear();
            
            for (auto const& row : res)
            {
                BuildMapping mapping;
                mapping.BuildNumber = row["build_number"].as<uint32>();
                mapping.PatchVersion = row["patch_version"].as<std::string>();
                mapping.ParserVersion = row["parser_version"].as<std::string>();
                
                _mappings[mapping.BuildNumber] = mapping;
            }

            LOG("Loaded {} supported builds from database", _mappings.size());
        }
        catch (std::exception const& e)
        {
            LOG("ERROR: Failed to load build info from database: {}", e.what());
            throw;
        }
    }

    std::optional<BuildMapping> BuildInfo::GetMapping(uint32 buildNumber) const
    {
        if (auto it = _mappings.find(buildNumber); it != _mappings.end())
            return it->second;
        return std::nullopt;
    }

    bool BuildInfo::IsSupported(uint32 buildNumber) const
    {
        return _mappings.find(buildNumber) != _mappings.end();
    }
}