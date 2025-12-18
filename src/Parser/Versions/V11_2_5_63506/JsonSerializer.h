#ifndef JSON_SERIALIZER_V11_2_5_63506_H
#define JSON_SERIALIZER_V11_2_5_63506_H

#include "Common/BaseJsonSerializer.h"

#include <nlohmann/json.hpp>

namespace PktParser::Versions::V11_2_5_63506
{
	using json = nlohmann::ordered_json;

	class JsonSerializer : public Common::BaseJsonSerializer
	{
		
	};
}

#endif
