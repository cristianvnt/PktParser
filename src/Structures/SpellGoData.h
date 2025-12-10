#ifndef SPELL_GO_DATA_H
#define SPELL_GO_DATA_H

#include "Misc/Define.h"
#include "Misc/WowGuid.h"
#include "Structures/Packed/SpellCastFixedData.h"
#include "Structures/Packed/SpellHealPrediction.h"
#include "Structures/Packed/RuneData.h"
#include "Structures/Packed/SpellMissStatus.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"

#include <vector>

namespace PktParser::Structures
{
	using WowGuid128 = PktParser::Misc::WowGuid128;

	struct SpellGoData
	{
		WowGuid128 CasterGUID;
		WowGuid128 CasterUnit;
		WowGuid128 CastID;
		WowGuid128 OriginalCastID;

		Packed::SpellCastFixedData FixedData;

		Packed::SpellHealPrediction HealPrediction;
		WowGuid128 BeaconGUID;

		uint32 HitTargetsCount;
		uint32 MissTargetsCount;
		uint32 HitStatusCount;
		uint32 MissStatusCount;
		uint32 RemainingPowerCount;
		bool HasRuneData;
		uint32 TargetPointsCount;

		std::vector<Packed::SpellMissStatus> MissStatus;
		SpellTargetData TargetData;
		std::vector<WowGuid128> HitTargets;
		std::vector<WowGuid128> MissTargets;
		std::vector<uint8> HitStatus;
		std::vector<uint32> RemainingPower;
		Packed::RuneData Runes;
		std::vector<uint8> RuneCooldowns;
		std::vector<TargetLocation> TargetPoints;
	};
}
#endif