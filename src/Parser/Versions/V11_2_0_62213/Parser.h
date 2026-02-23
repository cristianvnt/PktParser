#ifndef PARSER_V11_2_0_62213_H
#define PARSER_V11_2_0_62213_H

#include "Common/BaseVersionParser.h"
#include "Common/BaseJsonSerializer.h"
#include "Common/ParseResult.h"

namespace PktParser::Versions::V11_2_0_62213
{
	using BitReader = PktParser::Reader::BitReader;

	class Parser final : public Common::BaseVersionParser<Parser, Common::BaseJsonSerializer>
	{
	public:
		Parser();
        Common::ParseResult ParseSpellStart(BitReader& reader);
        Common::ParseResult ParseSpellGo(BitReader& reader);
	};
}

#endif