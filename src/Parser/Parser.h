#ifndef PARSER_H
#define PARSER_H

#include "Reader/BitReader.h"
#include "Opcodes.h"

namespace PktParser
{
	class Parser
	{
	public:
		static void ParseAuthChallenge(BitReader& reader);
		static void ParseSpellGo(BitReader& reader);
		static void ParseSpellGo_CHUNKED(BitReader& reader);
	};
}

#endif // PARSER_H