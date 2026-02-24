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
        w.BeginObject();
        WriteSpellDataFields(w, data);
		w.EndObject();
	}

    void BaseJsonSerializer::WriteSpellDataFields(JsonWriter &w, Structures::SpellCastData const &data) const
    {
        w.WriteString("CasterGUID", data.CasterGUID.ToHexString());

        if (data.CasterUnit.IsEmpty() == false && data.CasterUnit != data.CasterGUID)
            w.WriteString("CasterUnit", data.CasterUnit.ToHexString());

        w.WriteString("CastID", data.CastID.ToHexString());

        if (!data.OriginalCastID.IsEmpty())
            w.WriteString("OriginalCastID", data.OriginalCastID.ToHexString());

        w.WriteInt("SpellID", data.FixedData.SpellID);

		if (data.FixedData.Visual.SpellXSpellVisualID)
            w.WriteUInt("SpellXSpellVisualID", data.FixedData.Visual.SpellXSpellVisualID);
        if (data.FixedData.Visual.ScriptVisualID)
            w.WriteUInt("ScriptVisualID", data.FixedData.Visual.ScriptVisualID);
        if (data.FixedData.CastFlags)
            w.WriteUInt("CastFlags", data.FixedData.CastFlags);
        if (data.FixedData.CastFlagsEx)
            w.WriteUInt("CastFlagsEx", data.FixedData.CastFlagsEx);
        if (data.FixedData.CastFlagsEx2)
            w.WriteUInt("CastFlagsEx2", data.FixedData.CastFlagsEx2);
        if (data.FixedData.CastTime)
            w.WriteUInt("CastTime", data.FixedData.CastTime);
        if (data.FixedData.MissileTrajectory.TravelTime)
            w.WriteUInt("TravelTime", data.FixedData.MissileTrajectory.TravelTime);
        if (data.FixedData.MissileTrajectory.Pitch != 0.0)
            w.WriteDouble("Pitch", data.FixedData.MissileTrajectory.Pitch);
        if (data.FixedData.AmmoDisplayID)
            w.WriteInt("AmmoDisplayID", data.FixedData.AmmoDisplayID);
        if (data.FixedData.DestLocSpellCastIndex)
            w.WriteUInt("DestLocSpellCastIndex", data.FixedData.DestLocSpellCastIndex);
        if (data.FixedData.Immunities.School)
            w.WriteUInt("ImmunitySchool", data.FixedData.Immunities.School);
        if (data.FixedData.Immunities.Value)
            w.WriteUInt("ImmunityValue", data.FixedData.Immunities.Value);
        if (data.HealPrediction.Points)
            w.WriteUInt("HealPoints", data.HealPrediction.Points);
        if (data.HealPrediction.Type)
            w.WriteUInt("HealType", data.HealPrediction.Type);
        if (!data.BeaconGUID.IsEmpty())
            w.WriteString("BeaconGUID", data.BeaconGUID.ToHexString());

		w.Key("Target");
		WriteTargetData(w, data.TargetData);

        if (!data.HitTargets.empty())
        {
            w.Key("HitTargets");
            w.BeginArray();
            for (size_t i = 0; i < data.HitTargets.size(); ++i)
            {
                w.BeginObject();
                WriteGuidTargetFields(w, data.HitTargets[i]);
                if (i < data.HitStatus.size() && data.HitStatus[i].Reason != 0)
                    w.WriteUInt("HitStatus", data.HitStatus[i].Reason);
                w.EndObject();
            }
            w.EndArray();
        }

        if (!data.MissTargets.empty())
        {
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
        }

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
        WriteTargetDataFields(w, target);
        w.EndObject();
    }

    void BaseJsonSerializer::WriteTargetDataFields(JsonWriter &w, Structures::SpellTargetData const &target) const
    {
        if (target.Flags)
        {
            w.WriteUInt("Flags", target.Flags);
            w.WriteString("FlagsString", GetTargetFlagName(target.Flags));
        }
        if (!target.Unit.IsEmpty())
            w.WriteString("Unit", target.Unit.ToHexString());
        if (!target.Item.IsEmpty())
            w.WriteString("Item", target.Item.ToHexString());

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

        if (!target.Name.empty())
            w.WriteString("Name", target.Name);
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
        w.WriteString("GUID", guid.ToHexString());
    }

    void BaseJsonSerializer::WriteTargetLocation(JsonWriter& w, TargetLocation const& loc)
    {
        w.BeginObject();
        if (!loc.Transport.IsEmpty())
            w.WriteString("Transport", loc.Transport.ToHexString());
        w.WriteDouble("X", loc.X);
        w.WriteDouble("Y", loc.Y);
        w.WriteDouble("Z", loc.Z);
        w.EndObject();
    }
}