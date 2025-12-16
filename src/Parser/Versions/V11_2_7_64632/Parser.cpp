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
	void Parser::ParseSpellTargetData(BitReader& reader, Structures::SpellTargetData& targetData)
	{
		targetData.Flags = reader.ReadUInt32();
		targetData.Unit = Misc::ReadPackedGuid128(reader);
		targetData.Item = Misc::ReadPackedGuid128(reader);

		targetData.Unknown1127_1 = Misc::ReadPackedGuid128(reader);
		targetData.Unknown1127_2 = reader.ReadBit();

		bool hasSrc = reader.ReadBit();
		bool hasDst = reader.ReadBit();
		bool hasOrientation = reader.ReadBit();
		bool hasMapID = reader.ReadBit();
		uint32 nameLength = reader.ReadBits(7);

		reader.ResetBitReader();

		if (hasSrc)
			targetData.SrcLocation = ReadLocation(reader);

		if (hasDst)
			targetData.DstLocation = ReadLocation(reader);

		if (hasOrientation)
			targetData.Orientation = reader.ReadFloat();

		if (hasMapID)
			targetData.MapID = reader.ReadUInt32();

		targetData.Name = reader.ReadWoWString(nameLength);
	}

	JsonSerializer* Parser::GetSerializer()
	{
		static JsonSerializer serializer;
		return &serializer;
	}

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