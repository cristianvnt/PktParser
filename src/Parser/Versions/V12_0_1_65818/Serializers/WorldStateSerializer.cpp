#include "WorldStateSerializer.h"

namespace PktParser::V12_0_1_65818::Serializers
{
    void SerializeUpdateWorldState(JsonWriter &w, WorldStateData const &data)
    {
        w.BeginObject();
        w.WriteInt("WorldStateId", data.Info.VariableID);
        w.WriteInt("Value", data.Info.Value);
        w.WriteBool("Hidden", data.Hidden);
        w.EndObject();
    }
}