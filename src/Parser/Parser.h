#ifndef PARSER_H
#define PARSER_H

#include "Reader/BitReader.h"
#include "Opcodes.h"
#include "JsonSerializer.h"
#include "Structures/SpellTargetData.h"

namespace PktParser
{
	class PktRouter;

	using BitReader = PktParser::Reader::BitReader;
	namespace Structs = PktParser::Structures;

	class Parser
	{
	public:
		static void RegisterHandlers(PktRouter& router);
		
		// parsers
		static json ParseAuthChallenge(BitReader& reader, uint32 pktNumber);
		static json ParseSpellGo(BitReader& reader, uint32 pktNumber);
		static json ParseUpdateWorldState(BitReader& reader, uint32 pktNumber);

		static void ParseSpellTargetData(BitReader& reader, uint32 spellID, Structs::SpellTargetData& targetData);
	};
}

#endif // !PARSER_H