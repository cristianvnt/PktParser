#ifndef OPCODES_H
#define OPCODES_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <fmt/core.h>

#include "Misc/Define.h"

namespace PktParser
{
	enum class Opcode : uint32
	{
		NULL_OPCODE							= 0,
		SMSG_AUTH_CHALLENGE					= 0x3D0000,
		CMSG_AUTH_SESSION					= 0x350001,
		SMSG_SPELL_START					= 0x4D002B,
		SMSG_SPELL_GO						= 0x4D002A,
		SMSG_UPDATE_WORLD_STATE				= 0x3601E4
	};

	static std::unordered_map<uint32, std::string> const OpcodeNames =
	{
		{0x3D0000, "SMSG_AUTH_CHALLENGE"},
		{0x350001, "CMSG_AUTH_SESSION"},
		{0x4D002B, "SMSG_SPELL_START"},
		{0x4D002A, "SMSG_SPELL_GO"},
		{0x3601E4, "SMSG_UPDATE_WORLD_STATE"}
	};

	inline std::string GetOpcodeName(uint32 opcode)
	{
		if (auto it = OpcodeNames.find(opcode); it != OpcodeNames.end())
			return it->second;

		return fmt::format("UNKNOWN OPCODE 0x{:06X}", opcode);
	}
}

#endif // !OPCODES_H
