#include "pchdef.h"
#include "BaseJsonSerializer.h"

using namespace PktParser::Enums;
using namespace PktParser::Misc;
using namespace PktParser::Reader;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;

namespace PktParser::Common
{
    json BaseJsonSerializer::SerializeFullPacket(PktHeader const& header, char const* opcodeName, 
        uint32 build, uint32 pktNumber, json&& packetData) const
    {
        json j;
        j["Number"] = pktNumber;
        j["Header"] = SerializePacketHead(header, opcodeName, build);
        j["Data"] = std::move(packetData);
        return j;
    }
    
    json BaseJsonSerializer::SerializePacketHead(PktHeader const& header, char const* opcodeName, uint32 build) const
    {
        json j;
        j["Direction"] = Misc::DirectionToString(header.direction);
        j["PacketName"] = opcodeName;
        j["ConnectionIndex"] = header.connectionIndex;
        j["TickCount"] = header.tickCount;
        j["Timestamp"] = Misc::FormatUnixMilliseconds(header.timestamp);
        j["Opcode"] = fmt::format("0x{:06X}", header.opcode);
        j["Length"] = header.packetLength - 4;
        j["Build"] = build;
        return j;
    }

    json BaseJsonSerializer::SerializeSpellGo(SpellGoData const& data) const
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
		hitArray.get_ref<json::array_t&>().reserve(data.HitTargets.size());
		for (size_t i = 0; i < data.HitTargets.size(); ++i)
		{
			json t = SerializeGuidTarget(data.HitTargets[i]);
			if (i < data.HitStatus.size())
				t["HitStatus"] = data.HitStatus[i].Reason;
			hitArray.emplace_back(std::move(t));
		}
		j["HitTargets"] = std::move(hitArray);

		json missArray = json::array();
		missArray.get_ref<json::array_t&>().reserve(data.MissTargets.size());
		for (size_t i = 0; i < data.MissTargets.size(); ++i)
		{
			json t = SerializeGuidTarget(data.MissTargets[i]);
			if (i < data.MissStatus.size())
			{
				t["MissReason"] = data.MissStatus[i].MissReason;
				t["ReflectStatus"] = data.MissStatus[i].ReflectStatus;
			}
			missArray.emplace_back(std::move(t));
		}
		j["MissTargets"] = std::move(missArray);

		if (!data.RemainingPower.empty())
		{
			json powerArray = json::array();
			powerArray.get_ref<json::array_t&>().reserve(data.RemainingPower.size());
			for (const auto& power : data.RemainingPower)
			{
				json powerObj;
				powerObj["Cost"] = power.Cost;
				powerObj["Type"] = power.Type;
				powerArray.emplace_back(std::move(powerObj));
			}
			j["RemainingPower"] = std::move(powerArray);
		}
    	
		if (data.HasRuneData)
		{
			json runeData;
			runeData["Start"] = data.Runes.Start;
			runeData["Count"] = data.Runes.Count;
			runeData["CooldownCount"] = data.RuneCooldowns.size();
			runeData["Cooldowns"] = data.RuneCooldowns;
			j["RuneData"] = std::move(runeData);
		}

		if (!data.TargetPoints.empty())
		{
			json pointsArray = json::array();
			pointsArray.get_ref<json::array_t&>().reserve(data.TargetPoints.size());
			for (auto const& point : data.TargetPoints)
				pointsArray.emplace_back(SerializeTargetLocation(point));
			j["TargetPoints"] = std::move(pointsArray);
		}

		return j;
	}

    json BaseJsonSerializer::SerializeTargetData(SpellTargetData const& target) const
	{
        json j;
        j["Flags"] = target.Flags;
        j["FlagsString"] = GetTargetFlagName(target.Flags);
        j["Unit"] = target.Unit.ToString();
        j["Item"] = target.Item.ToString();

        if (target.SrcLocation)
            j["SrcLocation"] = SerializeTargetLocation(*target.SrcLocation);

        if (target.DstLocation)
            j["DstLocation"] = SerializeTargetLocation(*target.DstLocation);

        if (target.Orientation)
            j["Orientation"] = *target.Orientation;

        if (target.MapID)
            j["MapID"] = *target.MapID;

        j["Name"] = target.Name;
        return j;
    }

    json BaseJsonSerializer::SerializeAuthChallenge(AuthChallengeData const* data)
    {
        json j;
        j["DosChallenge"] = json::array();
        j["DosChallenge"].get_ref<json::array_t&>().reserve(8);
        for (int i = 0; i < 8; i++)
            j["DosChallenge"].emplace_back(data->DosChallenge[i]);

        std::string challengeHex;
        challengeHex.reserve(64);
        for (int i = 0; i < 32; i++)
            fmt::format_to(std::back_inserter(challengeHex), "{:02X}", data->Challenge[i]);
        
        j["Challenge"] = std::move(challengeHex);
        j["DosZeroBits"] = data->DosZeroBits;
        return j;
    }

    json BaseJsonSerializer::SerializeUpdateWorldState(WorldStateInfo const* info, bool hidden)
    {
        json j;
        j["WorldStateId"] = info->VariableID;
        j["Value"] = info->Value;
        j["Hidden"] = hidden;
        return j;
    }

    // helpers
    json BaseJsonSerializer::SerializeGuidTarget(WowGuid128 const& guid)
    {
        json t;
        t["GUID"] = guid.ToString();
        t["Type"] = GuidTypeToString(guid.GetType());
        t["Low"] = guid.GetLow();
        if (guid.HasEntry())
            t["Entry"] = guid.GetEntry();
        return t;
    }

    json BaseJsonSerializer::SerializeTargetLocation(TargetLocation const& loc)
    {
        json j;
        j["Transport"] = loc.Transport.ToString();
        j["X"] = loc.X;
        j["Y"] = loc.Y;
        j["Z"] = loc.Z;
        return j;
    }
}