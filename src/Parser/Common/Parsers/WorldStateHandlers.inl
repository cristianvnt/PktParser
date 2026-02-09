#ifndef WORLDSTATE_HANDLERS_INL
#define WORLDSTATE_HANDLERS_INL

#include "Reader/BitReader.h"
#include "Structures/Packed/WorldStateInfo.h"
#include <nlohmann/json.hpp>

namespace PktParser::Common::Parsers::WorldStateHandlers
{
    using BitReader = PktParser::Reader::BitReader;
    using json = nlohmann::json;

    template <typename TSerializer>
    inline json ParseUpdateWorldStateDefault(BitReader& reader, TSerializer* serializer)
    {
        auto const* worldStateInfo = reader.ReadChunk<Structures::Packed::WorldStateInfo>();
        reader.ResetBitReader();
        bool hidden = reader.ReadBit();

        return serializer->SerializeUpdateWorldState(worldStateInfo, hidden);
    }
}

#endif