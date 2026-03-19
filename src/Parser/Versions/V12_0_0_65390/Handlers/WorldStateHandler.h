#pragma once

#include "Reader/BitReader.h"
#include "../Structures/WorldStateData.h"

namespace PktParser::V12_0_0_65390::Handlers
{
    using BitReader = PktParser::Reader::BitReader;

    Structures::WorldStateData ParseUpdateWorldState(BitReader& reader);
}