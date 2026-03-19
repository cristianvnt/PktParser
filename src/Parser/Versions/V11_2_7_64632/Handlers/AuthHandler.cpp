#include "AuthHandler.h"

using namespace PktParser::V11_2_7_64632::Structures;

namespace PktParser::V11_2_7_64632::Handlers
{
    AuthChallengeData ParseAuthChallengeData(BitReader &reader)
    {
        return *reader.ReadChunk<AuthChallengeData>();
    }
}
