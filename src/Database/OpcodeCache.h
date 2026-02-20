#ifndef OPCODE_CACHE_H
#define OPCODE_CACHE_H

#include "Misc/Define.h"
#include <string>
#include <unordered_map>

namespace PktParser::Db
{
    struct OpcodeInfo
    {
        uint32 Value;
        std::string Name;
        std::string Direction;
    };

    class OpcodeCache
    {
    private:
        OpcodeCache() = default;
        
        std::unordered_map<std::string, std::unordered_map<uint32, OpcodeInfo>> _cache;

    public:
        static OpcodeCache& Instance();

        void EnsureLoaded(std::string const& parserVersion);

        char const* GetOpcodeName(std::string const& parserVersion, uint32 opcodeValue) const;
        size_t GetOpcodeCount(std::string const& parserVersion) const;
        bool IsLoaded(std::string const& parserVersion) const;
    };
}

#endif