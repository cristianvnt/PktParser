#ifndef JSON_SERIALIZER_V11_2_7_64877_H
#define JSON_SERIALIZER_V11_2_7_64877_H

#include "Reader/PktFileReader.h"
#include "Structures/SpellTargetData.h"
#include "Common/BaseJsonSerializer.h"
#include "Common/JsonWriter.h"

namespace PktParser::Versions::V11_2_7_64877
{
	class JsonSerializer : public Common::BaseJsonSerializer
	{
	public:
		void WriteTargetData(Common::JsonWriter& w, Structures::SpellTargetData const& target) const override;
	};
}

#endif
