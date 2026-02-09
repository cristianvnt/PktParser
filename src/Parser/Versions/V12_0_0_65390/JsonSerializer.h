#ifndef JSON_SERIALIZER_V12_0_0_65390_H
#define JSON_SERIALIZER_V12_0_0_65390_H

#include "Reader/PktFileReader.h"
#include "Structures/SpellTargetData.h"
#include "Common/BaseJsonSerializer.h"

#include <nlohmann/json.hpp>

namespace PktParser::Versions::V12_0_0_65390
{
	using json = nlohmann::ordered_json;

	class JsonSerializer : public Common::BaseJsonSerializer
	{
	public:
		json SerializeTargetData(Structures::SpellTargetData const& target) const override;
	};
}

#endif
