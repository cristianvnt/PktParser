#ifndef AUTH_HANDLERS_INL
#define AUTH_HANDLERS_INL

#include "Reader/BitReader.h"
#include "Structures/Packed/AuthChallengeData.h"

namespace PktParser::Common::Parsers::AuthHandlers
{
    using BitReader = PktParser::Reader::BitReader;
    using json = nlohmann::json;

    template <typename TSerializer>
    inline json ParseAuthChallenge(BitReader& reader, TSerializer* serializer)
    {
        reader.ResetBitReader();
        
        Structures::Packed::AuthChallengeData const* authData = reader.ReadChunk<Structures::Packed::AuthChallengeData>();

        return serializer->SerializeAuthChallenge(authData);
    }
}

#endif