#include "WorldStateHandler.h"

using namespace PktParser::V11_2_7_64632::Structures;

namespace PktParser::V11_2_7_64632::Handlers
{
    WorldStateData ParseUpdateWorldState(BitReader &reader)
    {
        WorldStateData data{};
        data.Info = *reader.ReadChunk<WorldStateInfo>();
        reader.ResetBitReader();
        data.Hidden = reader.ReadBit();
        
        return data;
    }
}