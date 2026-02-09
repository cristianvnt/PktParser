#include "pchdef.h"
#include "Parser.h"

#include "Opcodes.h"
#include "RegisterHandlers.inl"
#include "Common/Parsers/SpellHandlers.inl"

using namespace PktParser::Common::Parsers;
using namespace PktParser::Versions;
using namespace PktParser::Db;

namespace PktParser::Versions::V11_2_5_63506
{
	Parser::Parser()
	{
		_registry.Reserve(REGISTRY_RESERVE_SIZE);
		RegisterAllHandlers(this, _registry);
	}

	json Parser::ParseSpellGo(BitReader& reader)
	{
		auto ParseSpellTargetData = [](BitReader& reader, Structures::SpellTargetData& targetData)
		{
			targetData.Flags = reader.ReadUInt32();
			targetData.Unit = Misc::ReadPackedGuid128(reader);
			targetData.Item = Misc::ReadPackedGuid128(reader);

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
		};

		return SpellHandlers::ParseSpellGoDefault(reader, &_serializer, ParseSpellTargetData);
	}
}