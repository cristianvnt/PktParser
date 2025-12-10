#ifndef CREATURE_IMMUNITIES_H
#define CREATURE_IMMUNITIES_H

#include "Misc/Define.h"

namespace PktParser::Structures::Packed
{
#pragma pack(push, 1)
	struct CreatureImmunities
	{
		uint32 School;
		uint32 Value;
	};
#pragma pack(pop)
}

#endif