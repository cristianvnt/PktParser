#ifndef AUTH_HANDLERS_INL
#define AUTH_HANDLERS_INL

#include "Reader/BitReader.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Common/JsonWriter.h"
#include "Common/ParseResult.h"

namespace PktParser::Common::Parsers::AuthHandlers
{
    using BitReader = PktParser::Reader::BitReader;

    template <typename TSerializer>
    inline ParseResult ParseAuthChallenge(BitReader& reader, TSerializer* serializer)
    {
        reader.ResetBitReader();
        
        Structures::Packed::AuthChallengeData const* authData = reader.ReadChunk<Structures::Packed::AuthChallengeData>();

        return ParseResult{ "", std::nullopt };
    }
}

#endif