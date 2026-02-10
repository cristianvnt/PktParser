#ifndef SPELL_GO_DATA_H
#define SPELL_GO_DATA_H

#include "Misc/Define.h"
#include "Misc/WowGuid.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"

#include <vector>

namespace PktParser::Structures
{
	using WowGuid128 = PktParser::Misc::WowGuid128;

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
		uint32 School;
		uint32 Value;
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

	struct SpellHealPrediction
	{
		uint32 Points;
		uint32 Type;
	};

	struct SpellHitStatus
	{
		uint8 Reason = 0;
	};

	struct SpellMissStatus
	{
		uint8 MissReason;
		uint8 ReflectStatus;
	};

	struct RuneData
	{
		uint8 Start;
		uint8 Count;
	};

	struct SpellPowerData
	{
		int8 Type = 0;
		int32 Cost = 0;
	};
#pragma pack(pop)

	struct SpellCastData
	{
		WowGuid128 CasterGUID;
		WowGuid128 CasterUnit;
		WowGuid128 CastID;
		WowGuid128 OriginalCastID;

		SpellCastFixedData FixedData;
		WowGuid128 BeaconGUID;
		SpellHealPrediction HealPrediction;

		uint32 HitTargetsCount;
		uint32 MissTargetsCount;
		uint32 HitStatusCount;
		uint32 MissStatusCount;
		uint32 RemainingPowerCount;
		bool HasRuneData;
		uint32 TargetPointsCount;

		SpellTargetData TargetData;
		std::vector<WowGuid128> HitTargets;
		std::vector<WowGuid128> MissTargets;
		std::vector<SpellHitStatus> HitStatus;
		std::vector<SpellMissStatus> MissStatus;
		std::vector<SpellPowerData> RemainingPower;
		RuneData Runes;
		std::vector<uint8> RuneCooldowns;
		std::vector<TargetLocation> TargetPoints;
	};
}
#endif