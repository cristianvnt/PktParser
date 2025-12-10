#ifndef WORLD_STATE_INFO_H
#define WORLD_STATE_INFO_H

#include "Misc/Define.h"

namespace PktParser::Structures::Packed
{
#pragma pack(push, 1)
	struct WorldStateInfo
	{
		int32 VariableID;
		int32 Value;
	};
#pragma pack(pop)
}

#endif // !WORLD_STATE_INFO_H