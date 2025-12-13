#ifndef PKT_HANDLER_H
#define PKT_HANDLER_H

#include "Misc/Define.h"
#include "Reader/BitReader.h"
#include "Opcodes.h"
#include "Reader/PktFileReader.h"

#include <functional>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace PktParser
{
	using json = nlohmann::ordered_json;
	using PktHandler = std::function<json(Reader::BitReader&, uint32)>;

	class PktRouter
	{
	private:
		std::unordered_map<uint32, PktHandler> _handlers;

	public:
		void RegisterHandler(Opcode opcode, PktHandler handler)
		{
			_handlers[static_cast<uint32>(opcode)] = std::move(handler);
		}

		json HandlePacket(uint32 opcode, Reader::BitReader& reader, uint32 pktNumber)
		{
			if (auto it = _handlers.find(opcode); it != _handlers.end())
				return it->second(reader, pktNumber);

			return json{};
		}
	};
}

#endif // !PKT_HANDLER_H
