#ifndef OPCODES_H
#define OPCODES_H

#include <unordered_map>
#include <string>

#include "Misc/Define.h"

namespace PktParser
{
	enum class Opcode : uint32
	{
		SMSG_AUTH_CHALLENGE					= 0x470000,
		CMSG_AUTH_SESSION					= 0x3F0001,
		SMSG_SPELL_START					= 0x4D002B,
		SMSG_SPELL_GO						= 0x4D002A
	};

	static std::unordered_map<uint32, std::string> const OpcodeNames =
	{
		{0x470000, "SMSG_AUTH_CHALLENGE"},
		{0x3F0001, "CMSG_AUTH_SESSION"},
		{0x4D002B, "SMSG_SPELL_START"},
		{0x4D002A, "SMSG_SPELL_GO"},
	};

	inline std::string GetOpcodeName(uint32 opcode)
	{
		if (OpcodeNames.contains(opcode))
			return OpcodeNames.at(opcode);

		return "UNKNOWN";
	}
}

#endif // !OPCODES_H
