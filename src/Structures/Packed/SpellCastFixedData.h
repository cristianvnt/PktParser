#ifndef SPELL_CAST_FIXED_DATA_H
#define SPELL_CAST_FIXED_DATA_H

#include "SpellCastVisual.h"
#include "MissileTrajectory.h"
#include "CreatureImmunities.h"

namespace PktParser::Structures::Packed
{
#pragma pack(push, 1)
	struct SpellCastFixedData
	{
		int32 SpellID;
		SpellCastVisual Visual;
		uint32 CastFlags;
		uint32 CastFlagsEx;
		uint32 CastFlagsEx2;
		uint32 CastTime;
		MissileTrajectoryResult MissileTrajectory;
		int32 AmmoDisplayID;
		uint8 DestLocSpellCastIndex;
		CreatureImmunities Immunities;
	};
#pragma pack(pop)
}

#endif