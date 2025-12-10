#ifndef RUNE_DATA_H
#define RUNE_DATA_H

#include "Misc/Define.h"

namespace PktParser::Structures::Packed
{
#pragma pack(push, 1)
	struct RuneData
	{
		uint8 Start;
		uint8 Count;
	};
#pragma pack(pop)
}

#endif