#pragma once

#include "Common/SpellSearchFields.h"
#include "../Structures/SpellCastData.h"

namespace PktParser::V11_2_0_62213::SearchFields
{
    Common::SpellSearchFields FillSpellFields(Structures::SpellCastData const& data);
}