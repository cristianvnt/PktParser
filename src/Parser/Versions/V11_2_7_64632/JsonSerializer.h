#ifndef JSON_SERIALIZER_V11_2_7_64632_H
#define JSON_SERIALIZER_V11_2_7_64632_H

#include "Reader/PktFileReader.h"
#include "Structures/SpellTargetData.h"
#include "Common/BaseJsonSerializer.h"

#include <nlohmann/json.hpp>

namespace PktParser::Versions::V11_2_7_64632
{
	using json = nlohmann::ordered_json;

	class JsonSerializer : public Common::BaseJsonSerializer
	{
	public:
		json SerializeTargetData(Structures::SpellTargetData const& target) const override;
	};
}

#endif
