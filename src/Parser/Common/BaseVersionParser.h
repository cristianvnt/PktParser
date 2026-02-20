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

        json ParseAuthChallenge(Reader::BitReader& reader)
        {
            return Parsers::AuthHandlers::ParseAuthChallenge(reader, &_serializer);
        }

        json ParseUpdateWorldState(Reader::BitReader& reader)
        {
            return Parsers::WorldStateHandlers::ParseUpdateWorldState(reader, &_serializer);
        }

        TSerializer* GetSerializer() { return &_serializer; }
    };
}

#endif