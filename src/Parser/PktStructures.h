#ifndef PACKET_STRUCTURES_H
#define PACKET_STRUCTURES_H

#include "Misc/Define.h"
#include "Misc/WowGuid.h"

#include <vector>

namespace PktParser
{
#pragma pack(push, 1)
	struct SpellCastVisual
	{
		int32 SpellXSpellVisualID;
		int32 ScriptVisualID;
	};

	struct MissileTrajectoryResult
	{
		uint32 TravelTime;
		float Pitch;
	};

	struct CreatureImmunities
	{
		int32 School;
		int32 Value;
	};

	struct SpellHealPrediction 
	{
		int32 Points;
		uint8 Type;
	};

	struct RuneData 
	{
		uint8 Start;
		uint8 Count;
	};

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

	struct TargetLocationData
	{
		float X;
		float Y;
		float Z;
	};
#pragma pack(pop)

	struct SpellHitStatus
	{
		WowGuid128 Target;
		uint8 HitResult;
	};

	struct SpellMissStatus
	{
		WowGuid128 Target;
		uint8 MissReason;
		uint8 ReflectStatus;
	};

	struct TargetLocation
	{
		WowGuid128 Transport;
		float X;
		float Y;
		float Z;
	};
}

#endif // !PACKET_STRUCTURES_H