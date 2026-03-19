#include "SpellSerializer.h"

using namespace PktParser::V11_2_0_62213::Structures;

namespace PktParser::V11_2_0_62213::Serializers
{
    void SerializeSpellData(JsonWriter& w, SpellCastData const& data)
    {
        w.BeginObject();
        
        w.WriteGuid("CasterGUID", data.CasterGUID);

        if (data.CasterUnit.IsEmpty() == false && data.CasterUnit != data.CasterGUID)
            w.WriteGuid("CasterUnit", data.CasterUnit);

        w.WriteGuid("CastID", data.CastID);

        if (!data.OriginalCastID.IsEmpty())
            w.WriteGuid("OriginalCastID", data.OriginalCastID);

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
        if (data.FixedData.MissileTrajectory.Pitch != 0.f)
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
            w.WriteGuid("BeaconGUID", data.BeaconGUID);

		w.Key("Target");
		SerializeTargetData(w, data.TargetData);

        if (!data.HitTargets.empty())
        {
            w.Key("HitTargets");
            w.BeginArray();
            for (size_t i = 0; i < data.HitTargets.size(); ++i)
            {
                w.BeginObject();
                w.WriteGuid("GUID", data.HitTargets[i]);
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
                w.WriteGuid("GUID", data.MissTargets[i]);
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
                SerializeTargetLocation(w, point);
            w.EndArray();
        }

        w.EndObject();
    }

    void SerializeTargetData(JsonWriter& w, SpellTargetData const& target)
    {
        w.BeginObject();
        if (target.Flags)
            w.WriteUInt("Flags", target.Flags);

        if (target.SrcLocation)
        {
            w.Key("SrcLocation");
            SerializeTargetLocation(w, *target.SrcLocation);
        }

        if (target.DstLocation)
        {
            w.Key("DstLocation");
            SerializeTargetLocation(w, *target.DstLocation);
        }

        if (target.Orientation)
            w.WriteDouble("Orientation", *target.Orientation);

        if (target.MapID)
            w.WriteUInt("MapID", *target.MapID);

        if (!target.Name.empty())
            w.WriteString("Name", target.Name);
        w.EndObject();
    }

    void SerializeTargetLocation(JsonWriter& w, Structures::TargetLocation const& loc)
    {
        w.BeginObject();
        if (!loc.Transport.IsEmpty())
            w.WriteGuid("Transport", loc.Transport);
        w.WriteDouble("X", loc.X);
        w.WriteDouble("Y", loc.Y);
        w.WriteDouble("Z", loc.Z);
        w.EndObject();
    }
}