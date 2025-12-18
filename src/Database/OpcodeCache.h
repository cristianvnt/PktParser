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
        
        bool _loaded = false;
        std::unordered_map<uint32, OpcodeInfo> _opcodes;

    public:
        static OpcodeCache& Instance();

        void LoadFromDatabase(std::string const& parserVersion);
        char const* GetOpcodeName(uint32 opcodeValue) const;
        
        bool IsLoaded() const { return _loaded; }
        size_t GetOpcodeCount() const { return _opcodes.size(); }
    };
}

#endif