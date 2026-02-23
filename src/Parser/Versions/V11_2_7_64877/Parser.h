#ifndef PARSER_V11_2_7_64877_H
#define PARSER_V11_2_7_64877_H

#include "Common/BaseVersionParser.h"
#include "JsonSerializer.h"
#include "Common/JsonWriter.h"
#include "Common/ParseResult.h"

namespace PktParser::Versions::V11_2_7_64877
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