#ifndef OPCODES_H
#define OPCODES_H

#include <unordered_map>
#include <string>

#include "Misc/Define.h"

namespace PktParser
{
	enum class Opcode : uint32
	{
		SMSG_AUTH_CHALLENGE					= 0x3D0000,
		CMSG_AUTH_SESSION					= 0x350001,
		SMSG_SPELL_START					= 0x4D002B,
		SMSG_SPELL_GO						= 0x4D002A,
		SMSG_UPDATE_WORLD_STATE				= 0x3601E4
	};

	static std::unordered_map<uint32, std::string> const OpcodeNames =
	{
		{0x3D0000, "SMSG_AUTH_CHALLENGE"},
		{0x3F0001, "CMSG_AUTH_SESSION"},
		{0x4D002B, "SMSG_SPELL_START"},
		{0x4D002A, "SMSG_SPELL_GO"},
		{0x3601E4, "SMSG_UPDATE_WORLD_STATE"}
	};

	inline std::string GetOpcodeName(uint32 opcode)
	{
		if (OpcodeNames.contains(opcode))
			return OpcodeNames.at(opcode);

		return "UNKNOWN";
	}
}

#endif // !OPCODES_H
