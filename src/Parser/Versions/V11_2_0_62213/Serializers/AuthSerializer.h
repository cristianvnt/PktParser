#pragma once

#include "JsonWriter.h"
#include "../Structures/AuthChallengeData.h"

using namespace PktParser::V11_2_0_62213::Structures;

namespace PktParser::V11_2_0_62213::Serializers
{
    using JsonWriter = PktParser::Common::JsonWriter;

    void SerializeAuthChallenge(JsonWriter& w, AuthChallengeData const& data);
}