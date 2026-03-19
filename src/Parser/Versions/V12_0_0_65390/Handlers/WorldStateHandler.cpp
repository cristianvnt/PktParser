#include "WorldStateHandler.h"

using namespace PktParser::V12_0_0_65390::Structures;

namespace PktParser::V12_0_0_65390::Handlers
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