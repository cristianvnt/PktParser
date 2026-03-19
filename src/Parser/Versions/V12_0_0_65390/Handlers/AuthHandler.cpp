#include "AuthHandler.h"

using namespace PktParser::V12_0_0_65390::Structures;

namespace PktParser::V12_0_0_65390::Handlers
{
    AuthChallengeData ParseAuthChallengeData(BitReader &reader)
    {
        return *reader.ReadChunk<AuthChallengeData>();
    }
}
