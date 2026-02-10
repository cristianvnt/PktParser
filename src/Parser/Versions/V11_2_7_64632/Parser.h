#ifndef PARSER_V11_2_7_64632_H
#define PARSER_V11_2_7_64632_H

#include <unordered_map>

#include "Common/BaseVersionParser.h"
#include "JsonSerializer.h"

namespace PktParser::Versions::V11_2_7_64632
{
	using BitReader = PktParser::Reader::BitReader;

	class Parser final : public Common::BaseVersionParser<Parser, JsonSerializer>
	{
	public:
		Parser();
		json ParseSpellStart(BitReader& reader);
        json ParseSpellGo(BitReader& reader);
	};
}

#endif