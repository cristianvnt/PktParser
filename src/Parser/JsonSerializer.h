#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include "Reader/PktFileReader.h"
#include "Misc/WowGuid.h"
#include "Misc/Utilities.h"
#include "Misc/Logger.h"
#include "Structures/SpellGoData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Structures/Packed/WorldStateInfo.h"

#include <nlohmann/json.hpp>
#include <string>
#include <fmt/core.h>

namespace PktParser
{
	using json = nlohmann::ordered_json;

	class JsonSerializer
	{
	public:

		static json SerializePacketHead(Reader::PktHeader const& header, uint32 build);

		static json SerializeAuthChallenge(Structures::Packed::AuthChallengeData const* data);

		static json SerializeSpellGo(Structures::SpellGoData const& data);

		static json SerializeTargetData(Structures::SpellTargetData const& target);

		static json SerializeUpdateWorldState(Structures::Packed::WorldStateInfo const* info, bool hidden);

		static json SerializeFullPacket(Reader::PktHeader const& header, uint32 build, uint32 pktNumber, json const& packetData);
	};
}

#endif
