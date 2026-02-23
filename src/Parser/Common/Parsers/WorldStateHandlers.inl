#ifndef WORLDSTATE_HANDLERS_INL
#define WORLDSTATE_HANDLERS_INL

#include "Reader/BitReader.h"
#include "Structures/Packed/WorldStateInfo.h"
#include "Common/JsonWriter.h"
#include "Common/ParseResult.h"

namespace PktParser::Common::Parsers::WorldStateHandlers
{
    using BitReader = PktParser::Reader::BitReader;

    template <typename TSerializer>
    inline ParseResult ParseUpdateWorldState(BitReader& reader, TSerializer* serializer)
    {
        auto const* worldStateInfo = reader.ReadChunk<Structures::Packed::WorldStateInfo>();
        reader.ResetBitReader();
        bool hidden = reader.ReadBit();

        JsonWriter w(128);
        serializer->WriteUpdateWorldState(w, worldStateInfo, hidden);

        return ParseResult{ w.TakeString(), std::nullopt };
    }
}

#endif