#ifndef PARSER_H
#define PARSER_H

#include "Reader/BitReader.h"
#include "Opcodes.h"

namespace PktParser
{
	class PktRouter;

	class Parser
	{
	public:
		static void RegisterHandlers(PktRouter& router);
		
		// parsers
		static void ParseAuthChallenge(BitReader& reader);
		static void ParseSpellGo(BitReader& reader);
		static void ParseUpdateWorldState(BitReader& reader);
	};
}

#endif // !PARSER_H