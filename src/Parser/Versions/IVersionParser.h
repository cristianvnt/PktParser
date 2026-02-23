#ifndef IVERSION_PARSER_H
#define IVERSION_PARSER_H

#include "Misc/Define.h"
#include "Common/ParseResult.h"
#include <optional>

namespace PktParser::Reader { class BitReader; }

namespace PktParser::Versions
{
    class IVersionParser
    {
    public:
        virtual ~IVersionParser() = default;
        virtual std::optional<Common::ParseResult> ParsePacket(uint32 opcode, Reader::BitReader& reader) = 0;
    };
}

#endif