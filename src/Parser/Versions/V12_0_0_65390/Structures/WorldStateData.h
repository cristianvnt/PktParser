#pragma once

#include "Misc/Define.h"

namespace PktParser::V12_0_0_65390::Structures
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
