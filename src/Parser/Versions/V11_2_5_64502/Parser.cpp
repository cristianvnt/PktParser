#include "Parser.h"
#include "Opcodes.h"

namespace PktParser::Versions::V11_2_5_64502
{
	ParserMethod Parser::GetParserMethod(uint32 opcode) const
	{
		switch(opcode)
		{
			case Opcodes::SMSG_AUTH_CHALLENGE:
				return &ParseAuthChallenge;
			case Opcodes::SMSG_SPELL_GO:
				return &ParseSpellGo;
			case Opcodes::SMSG_UPDATE_WORLD_STATE:
				return &ParseUpdateWorldState;
			case Opcodes::CMSG_AUTH_SESSION:
			case Opcodes::SMSG_SPELL_START:
				return nullptr;
			default:
				return nullptr;
		}
	}

	char const* Parser::GetOpcodeName(uint32 opcode) const
	{
		auto it = OpcodeNames.find(opcode);
		return it != OpcodeNames.end() ? it->second : "UNKNOWN_OPCODE";
	}
}