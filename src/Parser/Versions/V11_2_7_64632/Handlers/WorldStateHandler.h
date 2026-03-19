#pragma once

#include "Reader/BitReader.h"
#include "../Structures/WorldStateData.h"

namespace PktParser::V11_2_7_64632::Handlers
{
    using BitReader = PktParser::Reader::BitReader;

    Structures::WorldStateData ParseUpdateWorldState(BitReader& reader);
}