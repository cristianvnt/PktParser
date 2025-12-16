#ifndef JSON_SERIALIZER_V11_2_5_64502_H
#define JSON_SERIALIZER_V11_2_5_64502_H

#include "Common/BaseJsonSerializer.h"

#include <nlohmann/json.hpp>

namespace PktParser::Versions::V11_2_5_64502
{
	using json = nlohmann::ordered_json;

	class JsonSerializer : public Common::BaseJsonSerializer
	{
		
	};
}

#endif
