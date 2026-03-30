#pragma once

#include "Common/SpellSearchFields.h"
#include "../Structures/SpellCastData.h"

namespace PktParser::V12_0_0_65390::SearchFields
{
    Common::SpellSearchFields FillSpellFields(Structures::SpellCastData const& data);
}