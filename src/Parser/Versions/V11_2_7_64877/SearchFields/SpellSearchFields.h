#pragma once

#include "Common/ParseResult.h"
#include "../Structures/SpellCastData.h"

namespace PktParser::V11_2_7_64877::SearchFields
{
    Common::SpellSearchFields FillSpellFields(Structures::SpellCastData const& data);
}