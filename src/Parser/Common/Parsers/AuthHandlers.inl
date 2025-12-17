#ifndef AUTH_HANDLERS_INL
#define AUTH_HANDLERS_INL

#include "Reader/BitReader.h"
#include "Structures/Packed/AuthChallengeData.h"
#include <nlohmann/json.hpp>

namespace PktParser::Common::Parsers::AuthHandlers
{
    using BitReader = PktParser::Reader::BitReader;
    using json = nlohmann::json;

    template <typename TSerializer>
    inline json ParseAuthChallengeDefault(BitReader& reader, TSerializer* serializer)
    {
        reader.ReadUInt32(); // skip opcode
        reader.ResetBitReader();
        
        Structures::Packed::AuthChallengeData const* authData = reader.ReadChunk<Structures::Packed::AuthChallengeData>();

        return serializer->SerializeAuthChallenge(authData);
    }
}

#endif