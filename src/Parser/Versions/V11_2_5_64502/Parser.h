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
		static void ParseSpellTargetData(BitReader& reader, Structures::SpellTargetData& targetData);
		
		static JsonSerializer* GetSerializer();

	public:
		ParserMethod GetParserMethod(uint32 opcode) const override;
        char const* GetOpcodeName(uint32 opcode) const override;
        uint32 GetBuild() const override { return 64502; }
	};
}

#endif