#pragma once

#include "Reader/BitReader.h"
#include "../Structures/SpellCastData.h"
#include "../Structures/SpellTargetData.h"
#include "../Structures/TargetLocation.h"
#include "Misc/WowGuid.h"
#include "Common/JsonWriter.h"

namespace PktParser::V11_2_0_62213::Handlers
{
    using BitReader = PktParser::Reader::BitReader;

    Structures::SpellCastData ParseSpellCastData(BitReader& reader);

    Structures::TargetLocation ParseLocation(BitReader& reader);
    Structures::SpellTargetData ParseSpellTargetData(BitReader& reader);
}