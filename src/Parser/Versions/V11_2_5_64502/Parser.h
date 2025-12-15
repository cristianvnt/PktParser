#ifndef PARSER_V11_2_5_64502_H
#define PARSER_V11_2_5_64502_H

#include "Common/BaseParser.h"
#include "JsonSerializer.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"

namespace PktParser::Versions::V11_2_5_64502
{
	using BitReader = PktParser::Reader::BitReader;

	class Parser : public Common::BaseParser<Parser>
	{
		friend class Common::BaseParser<Parser>;

	private:
		static void ParseSpellTargetData(BitReader& reader, Structures::SpellTargetData& targetData)
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
				targetData.SrcLocation = ReadLocation(reader);

			if (hasDst)
				targetData.DstLocation = ReadLocation(reader);

			if (hasOrientation)
				targetData.Orientation = reader.ReadFloat();

			if (hasMapID)
				targetData.MapID = reader.ReadUInt32();

			targetData.Name = reader.ReadWoWString(nameLength);
		}
		
		static json SerializeAuthChallenge(Structures::Packed::AuthChallengeData const* data)
		{
			return JsonSerializer::SerializeAuthChallenge(data);
		}

		static json SerializeSpellGo(Structures::SpellGoData const& data)
		{
			return JsonSerializer::SerializeSpellGo(data);
		}

		static json SerializeUpdateWorldState(Structures::Packed::WorldStateInfo const* data, bool hidden)
		{
			return JsonSerializer::SerializeUpdateWorldState(data, hidden);
		}

	public:
		ParserMethod GetParserMethod(uint32 opcode) const override;
        char const* GetOpcodeName(uint32 opcode) const override;
        uint32 GetBuild() const override { return 64502; }
	};
}

#endif