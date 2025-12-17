#include "JsonSerializer.h"

#include "Structures/SpellTargetData.h"
#include "V11_2_5_64502/JsonSerializer.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;

namespace PktParser::Versions::V11_2_7_64632
{
	json JsonSerializer::SerializeTargetData(SpellTargetData const& target) const
	{
		json j = BaseJsonSerializer::SerializeTargetData(target);

		if (target.Unknown1127_1)
			j["Unknown1127_1"] = target.Unknown1127_1->ToString();
		if (target.Unknown1127_2)
			j["Unknown1127_2"] = *target.Unknown1127_2;

		return j;
	}
}