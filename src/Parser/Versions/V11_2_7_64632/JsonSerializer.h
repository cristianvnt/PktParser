#ifndef JSON_SERIALIZER_V11_2_7_64632_H
#define JSON_SERIALIZER_V11_2_7_64632_H

#include "IJsonSerializer.h"
#include "Reader/PktFileReader.h"
#include "Structures/SpellGoData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Structures/Packed/WorldStateInfo.h"
#include "Common/BaseJsonSerializer.h"

#include <nlohmann/json.hpp>

namespace PktParser::Versions::V11_2_7_64632
{
	using json = nlohmann::ordered_json;

	class JsonSerializer : public Common::BaseJsonSerializer
	{
	public:
		static json SerializeSpellGo(Structures::SpellGoData const& data);
		static json SerializeTargetData(Structures::SpellTargetData const& target);
	};
}

#endif
