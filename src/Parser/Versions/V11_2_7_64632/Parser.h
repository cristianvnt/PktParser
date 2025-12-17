#ifndef PARSER_V11_2_7_64632_H
#define PARSER_V11_2_7_64632_H

#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"
#include "JsonSerializer.h"
#include "IVersionParser.h"

namespace PktParser::Versions::V11_2_7_64632
{
	using BitReader = PktParser::Reader::BitReader;

	class Parser : public IVersionParser
	{
	public:
		static json ParseAuthChallenge(BitReader& reader);
        static json ParseUpdateWorldState(BitReader& reader);
        static json ParseSpellGo(BitReader& reader);
		
		static JsonSerializer* GetSerializer();

		ParserMethod GetParserMethod(uint32 opcode) const override;
        char const* GetOpcodeName(uint32 opcode) const override;
        uint32 GetBuild() const override { return 64632; }
	};
}

#endif