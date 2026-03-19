#pragma once

#include "Reader/BitReader.h"
#include "../Structures/AuthChallengeData.h"

namespace PktParser::V12_0_0_65390::Handlers
{
    using BitReader = PktParser::Reader::BitReader;

    Structures::AuthChallengeData ParseAuthChallengeData(BitReader& reader);
}