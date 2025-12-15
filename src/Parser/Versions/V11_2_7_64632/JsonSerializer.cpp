#include "JsonSerializer.h"
#include "Misc/Utilities.h"
#include "Misc/WowGuid.h"
#include "Structures/SpellGoData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Structures/Packed/WorldStateInfo.h"
#include "Opcodes.h"
#include "V11_2_5_64502/JsonSerializer.h"

using namespace PktParser::Versions::Common;
using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;

namespace PktParser::Versions::V11_2_7_64632
{
	json JsonSerializer::SerializeTargetData(SpellTargetData const& target)
	{
		json j = V11_2_5_64502::JsonSerializer::SerializeTargetData(target);

		if (target.Unknown1127_1)
			j["Unknown1127_1"] = target.Unknown1127_1->ToString();
		if (target.Unknown1127_2)
			j["Unknown1127_2"] = *target.Unknown1127_2;

		return j;
	}

	json JsonSerializer::SerializeSpellGo(SpellGoData const& data)
	{
		json j;

		j["CasterGUID"] = data.CasterGUID.ToString();
		j["CasterUnit"] = data.CasterUnit.ToString();
		j["CastID"] = data.CastID.ToString();
		j["OriginalCastID"] = data.OriginalCastID.ToString();

		j["SpellID"] = data.FixedData.SpellID;
		j["SpellXSpellVisualID"] = data.FixedData.Visual.SpellXSpellVisualID;
		j["ScriptVisualID"] = data.FixedData.Visual.ScriptVisualID;
		j["CastFlags"] = data.FixedData.CastFlags;
		j["CastFlagsEx"] = data.FixedData.CastFlagsEx;
		j["CastFlagsEx2"] = data.FixedData.CastFlagsEx2;
		j["CastTime"] = data.FixedData.CastTime;

		j["TravelTime"] = data.FixedData.MissileTrajectory.TravelTime;
		j["Pitch"] = data.FixedData.MissileTrajectory.Pitch;

		j["AmmoDisplayID"] = data.FixedData.AmmoDisplayID;
		j["DestLocSpellCastIndex"] = data.FixedData.DestLocSpellCastIndex;
		j["ImmunitySchool"] = data.FixedData.Immunities.School;
		j["ImmunityValue"] = data.FixedData.Immunities.Value;

		j["HealPoints"] = data.HealPrediction.Points;
		j["HealType"] = data.HealPrediction.Type;
		j["BeaconGUID"] = data.BeaconGUID.ToString();

		j["HitTargetsCount"] = data.HitTargetsCount;
		j["MissTargetsCount"] = data.MissTargetsCount;
		j["HitStatusCount"] = data.HitStatusCount;
		j["MissStatusCount"] = data.MissStatusCount;
		j["RemainingPowerCount"] = data.RemainingPowerCount;
		j["HasRuneData"] = data.HasRuneData;
		j["TargetPointsCount"] = data.TargetPointsCount;
		j["Target"] = SerializeTargetData(data.TargetData);

		json hitArray = json::array();
		for (size_t i = 0; i < data.HitTargets.size(); ++i)
		{
			json t;
			t["GUID"] = data.HitTargets[i].ToString();
			t["Type"] = GuidTypeToString(data.HitTargets[i].GetType());
			t["Low"] = data.HitTargets[i].GetLow();
			if (data.HitTargets[i].HasEntry())
				t["Entry"] = data.HitTargets[i].GetEntry();
			if (i < data.HitStatus.size())
				t["HitStatus"] = data.HitStatus[i];
			hitArray.push_back(t);
		}
		j["HitTargets"] = hitArray;

		json missArray = json::array();
		for (size_t i = 0; i < data.MissTargets.size(); ++i)
		{
			json t;
			t["GUID"] = data.MissTargets[i].ToString();
			t["Type"] = GuidTypeToString(data.MissTargets[i].GetType());
			t["Low"] = data.MissTargets[i].GetLow();
			if (i < data.MissStatus.size())
			{
				t["MissReason"] = data.MissStatus[i].MissReason;
				t["ReflectStatus"] = data.MissStatus[i].ReflectStatus;
			}
			missArray.push_back(t);
		}
		j["MissTargets"] = missArray;

		if (!data.RemainingPower.empty())
		{
			json powerArray = json::array();
			for (const auto& power : data.RemainingPower)
			{
				json powerObj;
				powerObj["Cost"] = power.Cost;
				powerObj["Type"] = power.Type;
				powerArray.push_back(powerObj);
			}
			j["RemainingPower"] = powerArray;
		}
    	
		if (data.HasRuneData)
		{
			json runeData;
			runeData["Start"] = data.Runes.Start;
			runeData["Count"] = data.Runes.Count;
			runeData["CooldownCount"] = data.RuneCooldowns.size();
			runeData["Cooldowns"] = data.RuneCooldowns;
			j["RuneData"] = runeData;
		}

		if (!data.TargetPoints.empty())
		{
			json pointsArray = json::array();
			for (auto const& point : data.TargetPoints)
			{
				json p;
				p["Transport"] = point.Transport.ToString();
				p["X"] = point.X;
				p["Y"] = point.Y;
				p["Z"] = point.Z;
				pointsArray.push_back(p);
			}
			j["TargetPoints"] = pointsArray;
		}

		return j;
	}
}