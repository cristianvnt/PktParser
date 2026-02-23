#include "pchdef.h"
#include "BaseJsonSerializer.h"

using namespace PktParser::Enums;
using namespace PktParser::Misc;
using namespace PktParser::Reader;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;

namespace PktParser::Common
{
    void BaseJsonSerializer::WriteSpellData(JsonWriter& w, SpellCastData const& data) const
	{
		w.WriteString("CasterGUID", data.CasterGUID.ToString());
		w.WriteString("CasterType", GuidTypeToString(data.CasterGUID.GetType()));
        w.WriteUInt("CasterEntry", data.CasterGUID.GetEntry());
        w.WriteUInt("CasterLow", data.CasterGUID.GetLow());
        w.WriteInt("MapID", data.CasterGUID.GetMapId());

		w.WriteString("CasterUnit", data.CasterUnit.ToString());
        w.WriteString("CastID", data.CastID.ToString());
        w.WriteString("OriginalCastID", data.OriginalCastID.ToString());

		w.WriteInt("SpellID", data.FixedData.SpellID);
        w.WriteUInt("SpellXSpellVisualID", data.FixedData.Visual.SpellXSpellVisualID);
        w.WriteUInt("ScriptVisualID", data.FixedData.Visual.ScriptVisualID);
        w.WriteUInt("CastFlags", data.FixedData.CastFlags);
        w.WriteUInt("CastFlagsEx", data.FixedData.CastFlagsEx);
        w.WriteUInt("CastFlagsEx2", data.FixedData.CastFlagsEx2);
        w.WriteUInt("CastTime", data.FixedData.CastTime);

		w.WriteUInt("TravelTime", data.FixedData.MissileTrajectory.TravelTime);
        w.WriteDouble("Pitch", data.FixedData.MissileTrajectory.Pitch);

        w.WriteInt("AmmoDisplayID", data.FixedData.AmmoDisplayID);
        w.WriteUInt("DestLocSpellCastIndex", data.FixedData.DestLocSpellCastIndex);
        w.WriteUInt("ImmunitySchool", data.FixedData.Immunities.School);
        w.WriteUInt("ImmunityValue", data.FixedData.Immunities.Value);

        w.WriteUInt("HealPoints", data.HealPrediction.Points);
        w.WriteUInt("HealType", data.HealPrediction.Type);
        w.WriteString("BeaconGUID", data.BeaconGUID.ToString());

        w.WriteUInt("HitTargetsCount", data.HitTargetsCount);
        w.WriteUInt("MissTargetsCount", data.MissTargetsCount);
        w.WriteUInt("HitStatusCount", data.HitStatusCount);
        w.WriteUInt("MissStatusCount", data.MissStatusCount);
        w.WriteUInt("RemainingPowerCount", data.RemainingPowerCount);
        w.WriteBool("HasRuneData", data.HasRuneData);
        w.WriteUInt("TargetPointsCount", data.TargetPointsCount);

		w.Key("Target");
		WriteTargetData(w, data.TargetData);

		w.Key("HitTargets");
        w.BeginArray();
        for (size_t i = 0; i < data.HitTargets.size(); ++i)
        {
            w.BeginObject();
            WriteGuidTargetFields(w, data.HitTargets[i]);
            if (i < data.HitStatus.size())
                w.WriteUInt("HitStatus", data.HitStatus[i].Reason);
            w.EndObject();
        }
        w.EndArray();

		w.Key("MissTargets");
        w.BeginArray();
        for (size_t i = 0; i < data.MissTargets.size(); ++i)
        {
            w.BeginObject();
            WriteGuidTargetFields(w, data.MissTargets[i]);
            if (i < data.MissStatus.size())
            {
                w.WriteUInt("MissReason", data.MissStatus[i].MissReason);
                w.WriteUInt("ReflectStatus", data.MissStatus[i].ReflectStatus);
            }
            w.EndObject();
        }
        w.EndArray();

		if (!data.RemainingPower.empty())
        {
            w.Key("RemainingPower");
            w.BeginArray();
            for (auto const& power : data.RemainingPower)
            {
                w.BeginObject();
                w.WriteInt("Cost", power.Cost);
                w.WriteInt("Type", power.Type);
                w.EndObject();
            }
            w.EndArray();
        }
    	
		if (data.HasRuneData)
        {
            w.Key("RuneData");
            w.BeginObject();
            w.WriteUInt("Start", data.Runes.Start);
            w.WriteUInt("Count", data.Runes.Count);
            w.WriteUInt("CooldownCount", data.RuneCooldowns.size());
            w.Key("Cooldowns");
            w.BeginArray();
            for (float cd : data.RuneCooldowns)
                w.Double(cd);
            w.EndArray();
            w.EndObject();
        }

        if (!data.TargetPoints.empty())
        {
            w.Key("TargetPoints");
            w.BeginArray();
            for (auto const& point : data.TargetPoints)
                WriteTargetLocation(w, point);
            w.EndArray();
        }
	}

    void BaseJsonSerializer::WriteTargetData(JsonWriter& w, SpellTargetData const& target) const
    {
        w.BeginObject();
        w.WriteUInt("Flags", target.Flags);
        w.WriteString("FlagsString", GetTargetFlagName(target.Flags));
        w.WriteString("Unit", target.Unit.ToString());
        w.WriteString("Item", target.Item.ToString());

        if (target.SrcLocation)
        {
            w.Key("SrcLocation");
            WriteTargetLocation(w, *target.SrcLocation);
        }

        if (target.DstLocation)
        {
			w.Key("DstLocation");
            WriteTargetLocation(w, *target.DstLocation);
        }

        if (target.Orientation)
            w.WriteDouble("Orientation", *target.Orientation);

        if (target.MapID)
            w.WriteUInt("MapID", *target.MapID);

        w.WriteString("Name", target.Name);
        w.EndObject();
    }

    void BaseJsonSerializer::WriteAuthChallenge(JsonWriter& w, AuthChallengeData const* data)
    {
        w.BeginObject();

        w.Key("DosChallenge");
        w.BeginArray();
        for (int i = 0; i < 8; i++)
			w.UInt(data->DosChallenge[i]);
        w.EndArray();

        std::string challengeHex;
        challengeHex.reserve(64);
        for (int i = 0; i < 32; i++)
            fmt::format_to(std::back_inserter(challengeHex), "{:02X}", data->Challenge[i]);
        
        w.WriteString("Challenge", challengeHex);
        w.WriteUInt("DosZeroBits", data->DosZeroBits);

        w.EndObject();
    }

    void BaseJsonSerializer::WriteUpdateWorldState(JsonWriter& w, WorldStateInfo const* info, bool hidden)
    {
        w.BeginObject();
        w.WriteInt("WorldStateId", info->VariableID);
        w.WriteInt("Value", info->Value);
        w.WriteBool("Hidden", hidden);
        w.EndObject();
    }

    // helpers
    void BaseJsonSerializer::WriteGuidTargetFields(JsonWriter& w, WowGuid128 const& guid)
    {
        w.WriteString("GUID", guid.ToString());
        w.WriteString("Type", GuidTypeToString(guid.GetType()));
        w.WriteUInt("Low", guid.GetLow());
        if (guid.HasEntry())
            w.WriteUInt("Entry", guid.GetEntry());
    }

    void BaseJsonSerializer::WriteTargetLocation(JsonWriter& w, TargetLocation const& loc)
    {
        w.BeginObject();
        w.WriteString("Transport", loc.Transport.ToString());
        w.WriteDouble("X", loc.X);
        w.WriteDouble("Y", loc.Y);
        w.WriteDouble("Z", loc.Z);
        w.EndObject();
    }
}