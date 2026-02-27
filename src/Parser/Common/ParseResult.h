#ifndef PARSE_RESULT_H
#define PARSE_RESULT_H

#include "Misc/Define.h"

#include <string>
#include <optional>
#include <variant>

namespace PktParser::Common
{
    struct SpellSearchFields
    {
        int32 spellId;
        std::string castId;
        std::string originalCastId;
        std::string casterGuid;
        std::string casterType;
        uint32 casterEntry;
        uint64 casterLow;
        int32 mapId;
    };

    using SearchFields = std::variant<SpellSearchFields>;

    struct ParseResult
    {
        std::string json;
        std::optional<SearchFields> searchFields;
    };
}

#endif