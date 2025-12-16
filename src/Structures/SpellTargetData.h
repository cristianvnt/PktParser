#ifndef SPELL_TARGET_DATA_H
#define SPELL_TARGET_DATA_H

#include "Misc/WowGuid.h"
#include "TargetLocation.h"

#include <string>
#include <optional>

namespace PktParser::Structures
{
    using WowGuid128 = PktParser::Misc::WowGuid128;

    struct SpellTargetData
    {
        uint32 Flags;
        WowGuid128 Unit;
        WowGuid128 Item;
        std::optional<WowGuid128> Unknown1127_1;
        std::optional<bool> Unknown1127_2;
        std::optional<TargetLocation> SrcLocation;
        std::optional<TargetLocation> DstLocation;
        std::optional<float> Orientation;
        std::optional<int32> MapID;
        std::string Name;
    };
}

#endif