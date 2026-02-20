#include "pchdef.h"
#include "OpcodeCache.h"
#include "Config.h"

namespace PktParser::Db
{
    OpcodeCache& OpcodeCache::Instance()
    {
        static OpcodeCache instance;
        return instance;
    }

    void OpcodeCache::EnsureLoaded(std::string const& parserVersion)
    {
        if (_cache.contains(parserVersion))
            return;

        try
        {
            std::string connStr = Config::GetPostgresConnectionString();
            
            pqxx::connection conn(connStr);
            pqxx::work txn(conn);
            
            std::string query = "SELECT opcode_value, opcode_name, direction "
                                "FROM opcodes "
                                "WHERE parser_version = " + txn.quote(parserVersion);
            
            pqxx::result res = txn.exec(query);
            
            auto& opcodes = _cache[parserVersion];
            
            for (auto const& row : res)
            {
                OpcodeInfo info;
                info.Value = row["opcode_value"].as<uint32>();
                info.Name = row["opcode_name"].as<std::string>();
                info.Direction = row["direction"].as<std::string>();
                
                opcodes[info.Value] = std::move(info);
            }
            
            LOG("Loaded {} opcodes from database for {}", opcodes.size(), parserVersion);
        }
        catch (std::exception const& e)
        {
            _cache.erase(parserVersion);
            LOG("ERROR: Failed to load opcodes for {}: {}", parserVersion, e.what());
            throw;
        }
    }
    
    char const* OpcodeCache::GetOpcodeName(std::string const& parserVersion, uint32 opcodeValue) const
    {
        auto versionIt = _cache.find(parserVersion);
        if (versionIt == _cache.end())
            return "UNKNOWN_VERSION";

        auto opcodeIt = versionIt->second.find(opcodeValue);
        if (opcodeIt != versionIt->second.end())
            return opcodeIt->second.Name.c_str();

        return "UNKNOWN_OPCODE";
    }
    
    size_t OpcodeCache::GetOpcodeCount(std::string const& parserVersion) const
    {
        auto it = _cache.find(parserVersion);
        if (it == _cache.end())
            return 0;
        return it->second.size(); 
    }

    bool OpcodeCache::IsLoaded(std::string const& parserVersion) const
    {
        return _cache.contains(parserVersion);
    }
}