#ifndef PARSER_V12_0_0_65390_H
#define PARSER_V12_0_0_65390_H

#include <unordered_map>

#include "Common/BaseVersionParser.h"
#include "JsonSerializer.h"

namespace PktParser::Versions::V12_0_0_65390
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