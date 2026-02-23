#include "pchdef.h"
#include "JsonSerializer.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;
using namespace PktParser::Versions;

namespace PktParser::Versions::V11_2_7_64877
{
	void JsonSerializer::WriteTargetData(Common::JsonWriter& w, Structures::SpellTargetData const& target) const
	{
		BaseJsonSerializer::WriteTargetData(w, target);

		w.WriteString("HousingGUID", target.HousingGUID.ToString());
		w.WriteBool("HousingIsResident", target.HousingIsResident);
	}
}