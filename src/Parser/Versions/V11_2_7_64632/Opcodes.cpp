#include "Opcodes.h"

namespace PktParser::Versions::V11_2_7_64632
{
    using namespace Opcodes;

	char const* GetOpcodeName(uint32 opcode)
    {
        switch (opcode)
        {
        case SMSG_AUTH_CHALLENGE:               return "SMSG_AUTH_CHALLENGE";
        case CMSG_AUTH_SESSION:                 return "CMSG_AUTH_SESSION";
        case SMSG_SPELL_START:                  return "SMSG_SPELL_START";
        case SMSG_SPELL_GO:                     return "SMSG_SPELL_GO";
        case SMSG_UPDATE_WORLD_STATE:           return "SMSG_UPDATE_WORLD_STATE";
        default:                                return "UNKNOWN_OPCODE";
        }
    }
}