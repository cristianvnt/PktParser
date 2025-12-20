#ifndef IVERSION_PARSER_H
#define IVERSION_PARSER_H

#include "Misc/Define.h"
#include <nlohmann/json.hpp>
#include <optional>

namespace PktParser::Reader { class BitReader; }

namespace PktParser::Versions
{
    using json = nlohmann::ordered_json;

    class IVersionParser
    {
    public:
        virtual ~IVersionParser() = default;

        virtual std::optional<json> ParsePacket(uint32 opcode, Reader::BitReader& reader) = 0;
        virtual char const* GetOpcodeName(uint32 opcode) const = 0;
    };
}

#endif