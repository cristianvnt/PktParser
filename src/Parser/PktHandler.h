#ifndef PKT_HANDLER_H
#define PKT_HANDLER_H

#include "Reader/BitReader.h"
#include "Parser/Opcodes.h"

#include <functional>
#include <unordered_map>

namespace PktParser
{
	using PktHandler = std::function<void(BitReader&)>;

	class PktRouter
	{
	private:
		std::unordered_map<uint32, PktHandler> _handlers;

	public:
		void RegisterHandler(Opcode opcode, PktHandler handler)
		{
			_handlers[static_cast<uint32>(opcode)] = handler;
		}

		bool HandlePacket(uint32 opcode, BitReader& reader)
		{
			auto it = _handlers.find(opcode);
			if (it == _handlers.end())
				return false;

			it->second(reader);
			return true;
		}
	};
}

#endif // !PKT_HANDLER_H
