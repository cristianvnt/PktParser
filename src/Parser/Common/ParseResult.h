#pragma once

#include "Misc/Define.h"
#include "SpellSearchFields.h"

#include <string>
#include <optional>
#include <variant>

namespace PktParser::Common
{
    using SearchFields = std::variant<SpellSearchFields>;

    struct ParseResult
    {
        std::string json;
        std::optional<SearchFields> searchFields;
    };
}
