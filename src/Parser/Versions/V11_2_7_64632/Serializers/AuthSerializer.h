#pragma once

#include "JsonWriter.h"
#include "../Structures/AuthChallengeData.h"

using namespace PktParser::V11_2_7_64632::Structures;

namespace PktParser::V11_2_7_64632::Serializers
{
    using JsonWriter = PktParser::Common::JsonWriter;

    void SerializeAuthChallenge(JsonWriter& w, AuthChallengeData const& data);
}