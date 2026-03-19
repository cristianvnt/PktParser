#pragma once

#include "JsonWriter.h"
#include "../Structures/SpellCastData.h"
#include "../Structures/SpellTargetData.h"
#include "../Structures/TargetLocation.h"

namespace PktParser::V12_0_1_65818::Serializers
{
    using JsonWriter = PktParser::Common::JsonWriter;

    static constexpr size_t SPELL_CAST_JSON_RESERVE = 8192;

    void SerializeSpellData(JsonWriter& w, Structures::SpellCastData const& data);
    void SerializeTargetData(JsonWriter& w, Structures::SpellTargetData const& target);
    void SerializeTargetLocation(JsonWriter& w, Structures::TargetLocation const& loc);
}
