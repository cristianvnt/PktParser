#include "WorldStateSerializer.h"

namespace PktParser::V11_2_0_62213::Serializers
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