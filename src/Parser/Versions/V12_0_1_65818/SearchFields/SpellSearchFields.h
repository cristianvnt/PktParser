#pragma once

#include "Common/SpellSearchFields.h"
#include "../Structures/SpellCastData.h"

namespace PktParser::V12_0_1_65818::SearchFields
{
    Common::SpellSearchFields FillSpellFields(Structures::SpellCastData const& data);
}