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

    void OpcodeCache::LoadFromDatabase(std::string const& parserVersion)
    {
        try
        {
            std::string connStr = Config::GetPostgresConnectionString();
            
            pqxx::connection conn(connStr);
            pqxx::work txn(conn);
            
            std::string query = "SELECT opcode_value, opcode_name, direction "
                                "FROM opcodes "
                                "WHERE parser_version = " + txn.quote(parserVersion);
            
            pqxx::result res = txn.exec(query);
            
            _opcodes.clear();
            
            for (auto const& row : res)
            {
                OpcodeInfo info;
                info.Value = row["opcode_value"].as<uint32>();
                info.Name = row["opcode_name"].as<std::string>();
                info.Direction = row["direction"].as<std::string>();
                
                _opcodes[info.Value] = info;
            }
            
            _loaded = true;
            
            LOG("Loaded {} opcodes from database for {}", _opcodes.size(), parserVersion);
        }
        catch (std::exception const& e)
        {
            LOG("ERROR: Failed to load opcodes from database: {}", e.what());
            throw;
        }
    }
    
    char const* OpcodeCache::GetOpcodeName(uint32 opcodeValue) const
    {
        if (auto it = _opcodes.find(opcodeValue); it != _opcodes.end())
            return it->second.Name.c_str();
        return "UNKNOWN_OPCODE";
    }
}