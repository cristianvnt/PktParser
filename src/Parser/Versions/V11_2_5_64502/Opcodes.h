#ifndef OPCODES_V11_2_5_64502_H
#define OPCODES_V11_2_5_64502_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <fmt/core.h>

#include "Misc/Define.h"

namespace PktParser::Versions::V11_2_5_64502
{
	namespace Opcodes
	{
		constexpr uint32 NULL_OPCODE						= 0;
		constexpr uint32 SMSG_AUTH_CHALLENGE				= 0x3D0000;
		constexpr uint32 CMSG_AUTH_SESSION					= 0x350001;
		constexpr uint32 SMSG_SPELL_START					= 0x4D002B;
		constexpr uint32 SMSG_SPELL_GO						= 0x4D002A;
		constexpr uint32 SMSG_UPDATE_WORLD_STATE			= 0x3601E4;
	};

	static std::unordered_map<uint32, char const*> const OpcodeNames =
	{
		{Opcodes::SMSG_AUTH_CHALLENGE, "SMSG_AUTH_CHALLENGE"},
		{Opcodes::CMSG_AUTH_SESSION, "CMSG_AUTH_SESSION"},
		{Opcodes::SMSG_SPELL_START, "SMSG_SPELL_START"},
		{Opcodes::SMSG_SPELL_GO, "SMSG_SPELL_GO"},
		{Opcodes::SMSG_UPDATE_WORLD_STATE, "SMSG_UPDATE_WORLD_STATE"}
	};
}

#endif
