#pragma once

#include "Reader/BitReader.h"
#include "IVersionParser.h"
#include "ParseResult.h"
#include "OpcodeRegistry.h"

namespace PktParser::V11_2_0_62213
{
	using BitReader = PktParser::Reader::BitReader;
	using IVersionParser = PktParser::Versions::IVersionParser;
	using ParseResult = PktParser::Common::ParseResult;

	class Parser final : public IVersionParser
	{
	private:
		Common::OpcodeRegistry<Parser> _registry;
		
	public:
		Parser();
		std::optional<ParseResult> ParsePacket(uint32 opcode, BitReader& reader) override;

		ParseResult HandleAuthChallenge(BitReader& reader);
        ParseResult HandleSpellStart(BitReader& reader);
        ParseResult HandleSpellGo(BitReader& reader);
		ParseResult HandleUpdateWorldState(BitReader& reader);
	};
}
