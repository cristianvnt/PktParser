#include "JsonSerializer.h"
#include "Misc/Utilities.h"
#include "Misc/WowGuid.h"
#include "Structures/SpellGoData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Structures/Packed/WorldStateInfo.h"
#include "Opcodes.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;
using namespace PktParser::Versions::Common;

namespace PktParser::Versions::V11_2_5_64502
{
	json JsonSerializer::SerializeTargetData(SpellTargetData const& target)
	{
        json j;
        j["Flags"] = target.Flags;
        j["FlagsString"] = Misc::GetTargetFlagName(target.Flags);
        j["Unit"] = target.Unit.ToString();
        j["Item"] = target.Item.ToString();

        if (target.SrcLocation)
        {
            j["SrcLocation"] =
            {
                {"Transport", target.SrcLocation->Transport.ToString()},
                {"X", target.SrcLocation->X},
                {"Y", target.SrcLocation->Y},
                {"Z", target.SrcLocation->Z}
            };
        }

        if (target.DstLocation)
        {
            j["DstLocation"] =
            {
                {"Transport", target.DstLocation->Transport.ToString()},
                {"X", target.DstLocation->X},
                {"Y", target.DstLocation->Y},
                {"Z", target.DstLocation->Z}
            };
        }

        if (target.Orientation)
            j["Orientation"] = *target.Orientation;

        if (target.MapID)
            j["MapID"] = *target.MapID;

        j["Name"] = target.Name;
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