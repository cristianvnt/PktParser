#include "pchdef.h"
#include "JsonSerializer.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;
using namespace PktParser::Versions;

namespace PktParser::Versions::V12_0_0_65390
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