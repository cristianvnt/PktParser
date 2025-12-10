#ifndef SPELL_HEAL_PREDICTION_H
#define SPELL_HEAL_PREDICTION_H

#include "Misc/Define.h"

namespace PktParser::Structures::Packed
{
#pragma pack(push, 1)
	struct SpellHealPrediction
	{
		int32 Points;
		int32 Type;
	};
#pragma pack(pop)
}

#endif