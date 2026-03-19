#include "AuthHandler.h"

using namespace PktParser::V12_0_1_65818::Structures;

namespace PktParser::V12_0_1_65818::Handlers
{
    AuthChallengeData ParseAuthChallengeData(BitReader &reader)
    {
        return *reader.ReadChunk<AuthChallengeData>();
    }
}
