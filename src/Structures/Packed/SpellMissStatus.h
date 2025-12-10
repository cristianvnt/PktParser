#ifndef SPELL_MISS_STATUS_H
#define SPELL_MISS_STATUS_H

#include "Misc/Define.h"

namespace PktParser::Structures::Packed
{
#pragma pack(push, 1)
	struct SpellMissStatus
	{
		uint8 MissReason;
		uint8 ReflectStatus;
	};
#pragma pack(pop)
}

#endif