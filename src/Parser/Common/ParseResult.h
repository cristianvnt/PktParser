#pragma once

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
        std::vector<uint32> hitTargetEntries;
    };

    using SearchFields = std::variant<SpellSearchFields>;

    struct ParseResult
    {
        std::string json;
        std::optional<SearchFields> searchFields;
    };
}
