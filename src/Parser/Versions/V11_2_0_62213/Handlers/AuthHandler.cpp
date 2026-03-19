#include "AuthHandler.h"

using namespace PktParser::V11_2_0_62213::Structures;

namespace PktParser::V11_2_0_62213::Handlers
{
    AuthChallengeData ParseAuthChallengeData(BitReader &reader)
    {
        return *reader.ReadChunk<AuthChallengeData>();
    }
}
