#ifndef PARSER_V12_0_0_65390_H
#define PARSER_V12_0_0_65390_H

#include <unordered_map>

#include "IVersionParser.h"
#include "JsonSerializer.h"
#include "Common/OpcodeRegistry.h"

namespace PktParser::Versions::V12_0_0_65390
{
	using BitReader = PktParser::Reader::BitReader;

	class Parser final : public IVersionParser
	{
	private:
		JsonSerializer _serializer;
		Common::OpcodeRegistry<Parser> _registry;

	public:
		Parser();
		
		std::optional<json> ParsePacket(uint32 opcode, BitReader& reader) override;
        char const* GetOpcodeName(uint32 opcode) const override;

		json ParseAuthChallenge(BitReader& reader);
		json ParseUpdateWorldState(BitReader& reader);
        json ParseSpellGo(BitReader& reader);
	};
}

#endif