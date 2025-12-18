#include "pchdef.h"
#include "Parser.h"

#include "Opcodes.h"
#include "ParserMacros.h"
#include "Database/OpcodeCache.h"

#include "Common/Parsers/SpellHandlers.inl"
#include "Common/Parsers/AuthHandlers.inl"
#include "Common/Parsers/WorldStateHandlers.inl"

using namespace PktParser::Reader;
using namespace PktParser::Versions;
using namespace PktParser::Misc;
using namespace PktParser::Db;
using namespace PktParser::Enums;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;
using namespace PktParser::Common::Parsers;

namespace PktParser::Versions::V11_2_7_64632
{
	json Parser::ParseAuthChallenge(BitReader& reader)
    {
        return AuthHandlers::ParseAuthChallengeDefault(reader, GetSerializer());
    }

	json Parser::ParseUpdateWorldState(BitReader& reader)
    {
        return WorldStateHandlers::ParseUpdateWorldStateDefault(reader, GetSerializer());
    }

	static void ParseSpellTargetData(BitReader& reader, SpellTargetData& targetData)
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
            targetData.SrcLocation = SpellHandlers::ReadLocation(reader);

        if (hasDst)
            targetData.DstLocation = SpellHandlers::ReadLocation(reader);

        if (hasOrientation)
            targetData.Orientation = reader.ReadFloat();

        if (hasMapID)
            targetData.MapID = reader.ReadUInt32();

        targetData.Name = reader.ReadWoWString(nameLength);
    }

	json Parser::ParseSpellGo(BitReader& reader)
	{
		return SpellHandlers::ParseSpellGoDefault(reader, GetSerializer(), ParseSpellTargetData);
	}

    char const* Parser::GetOpcodeName(uint32 opcode) const
    {
        return OpcodeCache::Instance().GetOpcodeName(opcode);
    }
}

IMPLEMENT_SERIALIZER(V11_2_7_64632, V11_2_7_64632::JsonSerializer);
BEGIN_PARSER_HANDLER(V11_2_7_64632)
	REGISTER_HANDLER(SMSG_AUTH_CHALLENGE, ParseAuthChallenge)
	REGISTER_HANDLER(SMSG_SPELL_GO, ParseSpellGo)
	REGISTER_HANDLER(SMSG_UPDATE_WORLD_STATE, ParseUpdateWorldState)
END_PARSER_HANDLER()