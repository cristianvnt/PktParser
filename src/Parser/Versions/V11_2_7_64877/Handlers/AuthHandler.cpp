#include "AuthHandler.h"

using namespace PktParser::V11_2_7_64877::Structures;

namespace PktParser::V11_2_7_64877::Handlers
{
    AuthChallengeData ParseAuthChallengeData(BitReader &reader)
    {
        return *reader.ReadChunk<AuthChallengeData>();
    }
}
