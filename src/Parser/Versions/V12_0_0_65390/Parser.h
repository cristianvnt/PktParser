#ifndef PARSER_V12_0_0_65390_H
#define PARSER_V12_0_0_65390_H

#include "Common/BaseVersionParser.h"
#include "JsonSerializer.h"
#include "Common/ParseResult.h"
#include "Common/JsonWriter.h"

namespace PktParser::Versions::V12_0_0_65390
{
	using BitReader = PktParser::Reader::BitReader;

	class Parser final : public Common::BaseVersionParser<Parser, JsonSerializer>
	{
	public:
		Parser();
		Common::ParseResult ParseSpellStart(BitReader& reader);
        Common::ParseResult ParseSpellGo(BitReader& reader);
	};
}

#endif