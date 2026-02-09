#ifndef BASE_VERSION_PARSER_H
#define BASE_VERSION_PARSER_H

#include "IVersionParser.h"
#include "OpcodeRegistry.h"
#include "Database/OpcodeCache.h"
#include "Parsers/AuthHandlers.inl"
#include "Parsers/WorldStateHandlers.inl"

namespace PktParser::Common
{
    template <typename TDerived, typename TSerializer>
    class BaseVersionParser : public Versions::IVersionParser
    {
    protected:
        TSerializer _serializer;
        OpcodeRegistry<TDerived> _registry;

    public:
        BaseVersionParser() : _registry { static_cast<TDerived*>(this) }
        {
        }

        std::optional<json> ParsePacket(uint32 opcode, Reader::BitReader& reader) override
        {
            reader.Skip(4);
            return _registry.Dispatch(opcode, reader);
        }

        char const* GetOpcodeName(uint32 opcode) const override
        {
            return Db::OpcodeCache::Instance().GetOpcodeName(opcode);
        }

        json ParseAuthChallenge(Reader::BitReader& reader)
        {
            return Parsers::AuthHandlers::ParseAuthChallengeDefault(reader, &_serializer);
        }

        json ParseUpdateWorldState(Reader::BitReader& reader)
        {
            return Parsers::WorldStateHandlers::ParseUpdateWorldStateDefault(reader, &_serializer);
        }

        TSerializer* GetSerializer() { return &_serializer; }
    };
}

#endif