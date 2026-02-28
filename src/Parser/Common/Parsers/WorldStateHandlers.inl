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
    inline ParseResult ParseUpdateWorldState(BitReader& reader, [[maybe_unused]] TSerializer* serializer)
    {
        reader.ReadChunk<Structures::Packed::WorldStateInfo>();
        reader.ResetBitReader();
        reader.ReadBit();

        return ParseResult{ "", std::nullopt };
    }
}

#endif