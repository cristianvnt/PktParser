#ifndef PKT_HANDLER_H
#define PKT_HANDLER_H

#include "Reader/BitReader.h"
#include "Parser/Opcodes.h"

#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace PktParser
{
	using PktHandler = json(*)(BitReader&);

	class PktRouter
	{
	private:
		std::unordered_map<uint32, PktHandler> _handlers;

	public:
		void RegisterHandler(Opcode opcode, PktHandler handler)
		{
			_handlers[static_cast<uint32>(opcode)] = handler;
		}

		json HandlePacket(uint32 opcode, BitReader& reader)
		{
			if (auto it = _handlers.find(opcode); it != _handlers.end())
				return it->second(reader);

			return json::object();
		}
	};
}

#endif // !PKT_HANDLER_H
