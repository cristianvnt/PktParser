#pragma once

#include "Misc/WowGuid.h"
#include "TargetLocation.h"

#include <string>
#include <optional>

namespace PktParser::V11_2_7_64632::Structures
{
    using WowGuid128 = PktParser::Misc::WowGuid128;

    struct SpellTargetData
    {
        uint32 Flags;
        WowGuid128 Unit;
        WowGuid128 Item;
        WowGuid128 HousingGUID;
        bool HousingIsResident;
        std::optional<TargetLocation> SrcLocation;
        std::optional<TargetLocation> DstLocation;
        std::optional<float> Orientation;
        std::optional<int32> MapID;
        std::string Name;
    };
}
