#pragma once

#include "Common/ParseResult.h"
#include "../Structures/SpellCastData.h"

namespace PktParser::V11_2_5_63506::SearchFields
{
    Common::SpellSearchFields FillSpellFields(Structures::SpellCastData const& data);
}