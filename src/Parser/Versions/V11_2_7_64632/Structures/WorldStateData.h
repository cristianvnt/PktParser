#pragma once

#include "Misc/Define.h"

namespace PktParser::V11_2_7_64632::Structures
{
#pragma pack(push, 1)
	struct WorldStateInfo
	{
		int32 VariableID;
		int32 Value;
	};
#pragma pack(pop)

	struct WorldStateData
	{
		WorldStateInfo Info;
		bool Hidden;
	};
}
