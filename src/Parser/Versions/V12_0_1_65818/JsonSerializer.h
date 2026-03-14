#ifndef JSON_SERIALIZER_V12_0_1_65818_H
#define JSON_SERIALIZER_V12_0_1_65818_H

#include "Reader/PktFileReader.h"
#include "Structures/SpellTargetData.h"
#include "Common/BaseJsonSerializer.h"
#include "Common/JsonWriter.h"

namespace PktParser::Versions::V12_0_1_65818
{
	class JsonSerializer : public Common::BaseJsonSerializer
	{
	public:
		void WriteTargetData(Common::JsonWriter& w, Structures::SpellTargetData const& target) const override;
	};
}

#endif
