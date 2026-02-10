#include "pchdef.h"
#include "JsonSerializer.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;
using namespace PktParser::Versions;

namespace PktParser::Versions::V11_2_7_64877
{
	json JsonSerializer::SerializeTargetData(SpellTargetData const& target) const
	{
		json j = BaseJsonSerializer::SerializeTargetData(target);

		j["HousingGUID"] = target.HousingGUID.ToString();
		j["HousingIsResident"] = target.HousingIsResident;

		return j;
	}
}