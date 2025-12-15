#ifndef JSON_SERIALIZER_V11_2_5_64502_H
#define JSON_SERIALIZER_V11_2_5_64502_H

#include "Reader/PktFileReader.h"
#include "Structures/SpellGoData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Structures/Packed/WorldStateInfo.h"
#include "Common/BaseJsonSerializer.h"

#include <nlohmann/json.hpp>

namespace PktParser::Versions::V11_2_5_64502
{
	using json = nlohmann::ordered_json;

	class JsonSerializer : public Common::BaseJsonSerializer
	{
	public:
		static json SerializeTargetData(Structures::SpellTargetData const& target);
		static json SerializeSpellGo(Structures::SpellGoData const& data);
	};
}

#endif
