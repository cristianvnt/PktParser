#pragma once

#include "Reader/BitReader.h"
#include "../Structures/AuthChallengeData.h"

namespace PktParser::V12_0_1_65818::Handlers
{
    using BitReader = PktParser::Reader::BitReader;

    Structures::AuthChallengeData ParseAuthChallengeData(BitReader& reader);
}