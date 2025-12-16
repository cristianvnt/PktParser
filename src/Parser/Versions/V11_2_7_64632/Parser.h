#ifndef PARSER_V11_2_7_64632_H
#define PARSER_V11_2_7_64632_H

#include "Common/BaseParser.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"
#include "JsonSerializer.h"

namespace PktParser::Versions::V11_2_7_64632
{
	using BitReader = PktParser::Reader::BitReader;

	class Parser : public Common::BaseParser<Parser>
	{
		friend class Common::BaseParser<Parser>;

	private:
		static void ParseSpellTargetData(BitReader& reader, Structures::SpellTargetData& targetData);

		static JsonSerializer* GetSerializer();

	public:
		ParserMethod GetParserMethod(uint32 opcode) const override;
        char const* GetOpcodeName(uint32 opcode) const override;
        uint32 GetBuild() const override { return 64632; }
	};
}

#endif