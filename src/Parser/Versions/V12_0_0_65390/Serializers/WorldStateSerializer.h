#pragma once

#include "JsonWriter.h"
#include "../Structures/WorldStateData.h"

using namespace PktParser::V12_0_0_65390::Structures;

namespace PktParser::V12_0_0_65390::Serializers
{
    using JsonWriter = PktParser::Common::JsonWriter;

    void SerializeUpdateWorldState(JsonWriter& w, WorldStateData const& data);
}