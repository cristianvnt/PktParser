#ifndef IVERSION_PARSER_H
#define IVERSION_PARSER_H

#include "Reader/BitReader.h"

#include <nlohmann/json.hpp>

namespace PktParser::Versions
{
    using json = nlohmann::ordered_json;
    using ParserMethod = json(*)(Reader::BitReader&);

    class IVersionParser
    {
    public:
        virtual ~IVersionParser() = default;
        virtual ParserMethod GetParserMethod(uint32 opcode) const = 0;
        virtual char const* GetOpcodeName(uint32 opcode) const = 0;
        virtual uint32 GetBuild() const = 0;
    };
}

#endif