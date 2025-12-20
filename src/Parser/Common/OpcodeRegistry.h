#ifndef OPCODE_REGISTRY_H
#define OPCODE_REGISTRY_H

#include "Misc/Define.h"
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <optional>

namespace PktParser::Reader{ class BitReader; }

namespace PktParser::Common
{
    using json = nlohmann::ordered_json;
    using BitReader = Reader::BitReader;
    
    template <typename TParser>
    class OpcodeRegistry
    {
    public:
        using HandlerFunc = json (TParser::*)(BitReader&);

        explicit OpcodeRegistry(TParser* parser) : _parser{ parser } {}

        void Reserve(size_t count)
        {
            _handlers.reserve(count);
        }

        void Register(uint32 opcode, HandlerFunc handler)
        {
            _handlers[opcode] = handler;
        }

        std::optional<json> Dispatch(uint32 opcode, BitReader& reader) const
        {
            typename std::unordered_map<uint32, HandlerFunc>::const_iterator it = _handlers.find(opcode);
            if (it == _handlers.end())
                return std::nullopt;

            return (_parser->*(it->second))(reader);
        }

    private:
        TParser* _parser;
        std::unordered_map<uint32, HandlerFunc> _handlers;
    };
}

#endif