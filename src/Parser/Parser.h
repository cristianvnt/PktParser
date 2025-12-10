#ifndef PARSER_H
#define PARSER_H

#include "Reader/BitReader.h"
#include "Opcodes.h"
#include "JsonSerializer.h"

namespace PktParser
{
	class PktRouter;

	class Parser
	{
	public:
		static void RegisterHandlers(PktRouter& router);
		
		// parsers
		static json ParseAuthChallenge(BitReader& reader);
		static json ParseSpellGo(BitReader& reader);
		static json ParseUpdateWorldState(BitReader& reader);
	};
}

#endif // !PARSER_H