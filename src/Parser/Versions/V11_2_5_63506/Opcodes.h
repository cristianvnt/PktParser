#ifndef OPCODES_V11_2_5_64502_H
#define OPCODES_V11_2_5_64502_H

#include "Misc/Define.h"

namespace PktParser::Versions::V11_2_5_63506
{
	namespace Opcodes
	{
		constexpr uint32 SMSG_AUTH_CHALLENGE				= 0x3D0000;
		constexpr uint32 CMSG_AUTH_SESSION					= 0x350001;
		constexpr uint32 SMSG_SPELL_START					= 0x4D002B;
		constexpr uint32 SMSG_SPELL_GO						= 0x4D002A;
		constexpr uint32 SMSG_UPDATE_WORLD_STATE			= 0x3601E4;
	};
	
	char const* GetOpcodeName(uint32 opcode);
}

#endif
