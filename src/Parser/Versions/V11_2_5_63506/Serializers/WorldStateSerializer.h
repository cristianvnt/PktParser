#pragma once

#include "JsonWriter.h"
#include "../Structures/WorldStateData.h"

using namespace PktParser::V11_2_5_63506::Structures;

namespace PktParser::V11_2_5_63506::Serializers
{
    using JsonWriter = PktParser::Common::JsonWriter;

    void SerializeUpdateWorldState(JsonWriter& w, WorldStateData const& data);
}