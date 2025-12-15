#ifndef PARSER_V11_2_5_64502_H
#define PARSER_V11_2_5_64502_H

#include "IVersionParser.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"

namespace PktParser::Versions::V11_2_5_64502
{
	using BitReader = PktParser::Reader::BitReader;

	class Parser : public IVersionParser
	{
	private:
		static json ParseAuthChallenge(BitReader& reader, uint32 pktNumber);
		static json ParseSpellGo(BitReader& reader, uint32 pktNumber);
		static json ParseUpdateWorldState(BitReader& reader, uint32 pktNumber);

		// helpers
		static void ParseSpellTargetData(BitReader& reader, uint32 spellID, Structures::SpellTargetData& targetData);
		static Structures::TargetLocation ReadLocation(BitReader& reader);

	public:
		ParserMethod GetParserMethod(uint32 opcode) const override;
        char const* GetOpcodeName(uint32 opcode) const override;
        uint32 GetBuild() const override { return 64502; }
	};
}

#endif