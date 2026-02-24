#ifndef PARSE_RESULT_H
#define PARSE_RESULT_H

#include "Misc/Define.h"

#include <string>
#include <optional>

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

    struct ParseResult
    {
        std::string json;
        std::optional<SpellSearchFields> spellFields;
        bool storeAsJson;
    };
}

#endif