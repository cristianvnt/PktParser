#include "WorldStateHandler.h"

using namespace PktParser::V12_0_1_65818::Structures;

namespace PktParser::V12_0_1_65818::Handlers
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