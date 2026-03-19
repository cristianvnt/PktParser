#include "SpellHandler.h"

using namespace PktParser::Reader;
using namespace PktParser::V11_2_7_64632::Structures;

namespace PktParser::V11_2_7_64632::Handlers
{
    SpellCastData ParseSpellCastData(BitReader& reader)
    {
        SpellCastData data{};

        data.CasterGUID = Misc::ReadPackedGuid128(reader);
        data.CasterUnit = Misc::ReadPackedGuid128(reader);
        data.CastID = Misc::ReadPackedGuid128(reader);
        data.OriginalCastID = Misc::ReadPackedGuid128(reader);

        data.FixedData = *reader.ReadChunk<SpellCastFixedData>();
        data.HealPrediction = *reader.ReadChunk<SpellHealPrediction>();
        data.BeaconGUID = Misc::ReadPackedGuid128(reader);

        reader.ResetBitReader();

        data.HitTargetsCount = reader.ReadBits(16);
        data.MissTargetsCount = reader.ReadBits(16);
        data.HitStatusCount = reader.ReadBits(16);
        data.MissStatusCount = reader.ReadBits(16);
        data.RemainingPowerCount = reader.ReadBits(9);
        data.HasRuneData = reader.ReadBit();
        data.TargetPointsCount = reader.ReadBits(16);

        data.TargetData = ParseSpellTargetData(reader);

        data.HitTargets.resize(data.HitTargetsCount);
        for (uint32 i = 0; i < data.HitTargetsCount; ++i)
            data.HitTargets[i] = Misc::ReadPackedGuid128(reader);

        data.MissTargets.resize(data.MissTargetsCount);
        for (uint32 i = 0; i < data.MissTargetsCount; ++i)
            data.MissTargets[i] = Misc::ReadPackedGuid128(reader);

        reader.ReadChunkArray(data.HitStatus, data.HitStatusCount);
        
        data.MissStatus.resize(data.MissStatusCount);
        for (uint32 i = 0; i < data.MissStatusCount; ++i)
        {
            data.MissStatus[i].MissReason = reader.ReadUInt8();
            if (data.MissStatus[i].MissReason == 11) // SPELL_MISS_REFLECT
                data.MissStatus[i].ReflectStatus = reader.ReadUInt8();
            else
                data.MissStatus[i].ReflectStatus = 0;
        }

        reader.ReadChunkArray(data.RemainingPower, data.RemainingPowerCount);

        if (data.HasRuneData)
        {
            data.Runes = *reader.ReadChunk<RuneData>();
            uint32 cooldownCount = reader.ReadUInt32();
            
            reader.ReadChunkArray(data.RuneCooldowns, cooldownCount);
        }

        data.TargetPoints.resize(data.TargetPointsCount);
        for (uint32 i = 0; i < data.TargetPointsCount; ++i)
            data.TargetPoints[i] = ParseLocation(reader);

        return data;
    }

    TargetLocation ParseLocation(BitReader& reader)
    {
        TargetLocation loc;
        loc.Transport = Misc::ReadPackedGuid128(reader);
        loc.X = reader.ReadFloat();
        loc.Y = reader.ReadFloat();
        loc.Z = reader.ReadFloat();

        return loc;
    }

    SpellTargetData ParseSpellTargetData(BitReader& reader)
    {
        reader.ResetBitReader();

        SpellTargetData targetData{};
        targetData.Flags = reader.ReadUInt32();

        targetData.Unit = Misc::ReadPackedGuid128(reader);
        targetData.Item = Misc::ReadPackedGuid128(reader);
        
        targetData.HousingGUID = Misc::ReadPackedGuid128(reader);
        targetData.HousingIsResident = reader.ReadBit();

        bool hasSrc = reader.ReadBit();
        bool hasDst = reader.ReadBit();
        bool hasOrientation = reader.ReadBit();
        bool hasMapID = reader.ReadBit();
        uint32 nameLength = reader.ReadBits(7);

        reader.ResetBitReader();

        if (hasSrc)
            targetData.SrcLocation = ParseLocation(reader);

        if (hasDst)
            targetData.DstLocation = ParseLocation(reader);

        if (hasOrientation)
            targetData.Orientation = reader.ReadFloat();

        if (hasMapID)
            targetData.MapID = reader.ReadUInt32();

        targetData.Name = reader.ReadWoWString(nameLength);

        return targetData;
    }
}
