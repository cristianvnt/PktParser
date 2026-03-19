#pragma once

#include "JsonWriter.h"
#include "../Structures/AuthChallengeData.h"

using namespace PktParser::V12_0_1_65818::Structures;

namespace PktParser::V12_0_1_65818::Serializers
{
    using JsonWriter = PktParser::Common::JsonWriter;

    void SerializeAuthChallenge(JsonWriter& w, AuthChallengeData const& data);
}