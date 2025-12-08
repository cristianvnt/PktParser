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

		template<typename T> // gotta check it
		static inline void ReadChunk(BitReader& reader, T& out)
		{
			static_assert(std::is_trivially_copyable_v<T>);

			uint8 const* p = reader.GetCurrentPtr();

			alignas(16) T tmp;
			std::memcpy(&tmp, p, sizeof(T));
			reader.Skip(sizeof(T));
			out = tmp;
		}
	};
}

#endif // PARSER_H