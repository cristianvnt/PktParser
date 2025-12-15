#include "Parser.h"
#include "Opcodes.h"
#include "JsonSerializer.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Enums;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;

namespace PktParser::Versions::V11_2_7_64632
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