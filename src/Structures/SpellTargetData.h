#ifndef SPELL_TARGET_DATA_H
#define SPELL_TARGET_DATA_H

#include "Misc/WowGuid.h"
#include "TargetLocation.h"

#include <string>

namespace PktParser::Structures
{
    using WowGuid128 = PktParser::Misc::WowGuid128;

    struct SpellTargetData
    {
        uint32 Flags;
        WowGuid128 Unit;
        WowGuid128 Item;

        bool HasSrcLocation;
        bool HasDstLocation;
        bool HasOrientation;
        bool HasMapID;

        TargetLocation SrcLocation;
        TargetLocation DstLocation;
        float Orientation;
        uint32 MapID;
        std::string Name;
    };
}

#endif