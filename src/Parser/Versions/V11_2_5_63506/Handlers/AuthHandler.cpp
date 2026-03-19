#include "AuthHandler.h"

using namespace PktParser::V11_2_5_63506::Structures;

namespace PktParser::V11_2_5_63506::Handlers
{
    AuthChallengeData ParseAuthChallengeData(BitReader &reader)
    {
        return *reader.ReadChunk<AuthChallengeData>();
    }
}
