#ifndef SPELL_MISS_TYPE_H
#define SPELL_MISS_TYPE_H

#include "Misc/Define.h"
#include <cstdint>

namespace PktParser
{
	enum class SpellMissType : uint8
	{
		None = 0,
		Miss = 1,
		Resist = 2,
		Dodge = 3,
		Parry = 4,
		Block = 5,
		Evade = 6,
		Immune1 = 7,
		Immune2 = 8,
		Deflect = 9,
		Absorb = 10,
		Reflect = 11
	};
}

#endif // !SPELL_MISS_TYPE_H
