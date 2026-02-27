#ifndef SPELL_HANDLERS_INL
#define SPELL_HANDLERS_INL

#include "Reader/BitReader.h"
#include "Structures/SpellCastData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"
#include "Misc/WowGuid.h"
#include "Enums/TargetVersions.h"
#include "Common/JsonWriter.h"
#include "Common/ParseResult.h"

namespace PktParser::Common::Parsers::SpellHandlers
{
    using BitReader = PktParser::Reader::BitReader;
    using SpellTargetVersion = Enums::SpellTargetVersion;

    inline Structures::TargetLocation ReadLocation(BitReader& reader)
    {
        Structures::TargetLocation loc;
        loc.Transport = Misc::ReadPackedGuid128(reader);
        loc.X = reader.ReadFloat();
        loc.Y = reader.ReadFloat();
        loc.Z = reader.ReadFloat();

        return loc;
    }

    template <SpellTargetVersion V = SpellTargetVersion::Base>
    inline void ParseSpellTargetData(BitReader& reader, Structures::SpellTargetData& targetData)
    {
        if constexpr (V < SpellTargetVersion::Base)
            targetData.Flags = reader.ReadBits(28);
        else
            targetData.Flags = reader.ReadUInt32();

        if constexpr (V > SpellTargetVersion::Lower)
        {
            targetData.Unit = Misc::ReadPackedGuid128(reader);
            targetData.Item = Misc::ReadPackedGuid128(reader);
        }

        if constexpr (V >= SpellTargetVersion::Housing)
        {
            targetData.HousingGUID = Misc::ReadPackedGuid128(reader);
            targetData.HousingIsResident = reader.ReadBit();
        }

        bool hasSrc = reader.ReadBit();
        bool hasDst = reader.ReadBit();
        bool hasOrientation = reader.ReadBit();
        bool hasMapID = reader.ReadBit();
        uint32 nameLength = reader.ReadBits(7);

        reader.ResetBitReader();

        if constexpr (V < SpellTargetVersion::Base)
        {
            targetData.Unit = Misc::ReadPackedGuid128(reader);
            targetData.Item = Misc::ReadPackedGuid128(reader);
        }

        if (hasSrc)
            targetData.SrcLocation = SpellHandlers::ReadLocation(reader);

        if (hasDst)
            targetData.DstLocation = SpellHandlers::ReadLocation(reader);

        if (hasOrientation)
            targetData.Orientation = reader.ReadFloat();

        if (hasMapID)
            targetData.MapID = reader.ReadUInt32();

        targetData.Name = reader.ReadWoWString(nameLength);
    };

    template <SpellTargetVersion V>
    inline void ReadSpellHealPrediction(BitReader& reader, Structures::SpellHealPrediction& healPrediction)
    {
        healPrediction.Points = reader.ReadUInt32();

        if constexpr (V > SpellTargetVersion::Lower)
            healPrediction.Type = reader.ReadUInt32();
        else
            healPrediction.Type = reader.ReadUInt8();
    }

    template<SpellTargetVersion V, typename TSerializer, typename ParseTargetDataFunc>
    inline ParseResult ParseSpellCastData(BitReader& reader, TSerializer* serializer, ParseTargetDataFunc parseTargetData)
    {
        Structures::SpellCastData data{};

        data.CasterGUID = Misc::ReadPackedGuid128(reader);
        data.CasterUnit = Misc::ReadPackedGuid128(reader);
        data.CastID = Misc::ReadPackedGuid128(reader);
        data.OriginalCastID = Misc::ReadPackedGuid128(reader);

        Structures::SpellCastFixedData const* basicInfo = reader.ReadChunk<Structures::SpellCastFixedData>();
        data.FixedData = *basicInfo;

        ReadSpellHealPrediction<V>(reader, data.HealPrediction);
        data.BeaconGUID = Misc::ReadPackedGuid128(reader);

        data.HitTargetsCount = reader.ReadBits(16);
        data.MissTargetsCount = reader.ReadBits(16);
        data.HitStatusCount = reader.ReadBits(16);
        data.MissStatusCount = reader.ReadBits(16);
        data.RemainingPowerCount = reader.ReadBits(9);
        data.HasRuneData = reader.ReadBit();
        data.TargetPointsCount = reader.ReadBits(16);

        reader.ResetBitReader();

        parseTargetData(reader, data.TargetData);

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
            Structures::RuneData const* runePtr = reader.ReadChunk<Structures::RuneData>();
            data.Runes = *runePtr;
            uint32 cooldownCount = reader.ReadUInt32();
            
            reader.ReadChunkArray(data.RuneCooldowns, cooldownCount);
        }

        data.TargetPoints.resize(data.TargetPointsCount);
        for (uint32 i = 0; i < data.TargetPointsCount; ++i)
            data.TargetPoints[i] = ReadLocation(reader);

        JsonWriter w(2048);
        serializer->WriteSpellData(w, data);

        SpellSearchFields fields;
        fields.spellId = data.FixedData.SpellID;
        fields.castId = data.CastID.ToString();
        fields.originalCastId = data.OriginalCastID.ToString();
        fields.casterGuid = data.CasterGUID.ToString();
        fields.casterType = Enums::GuidTypeToString(data.CasterGUID.GetType());
        fields.casterEntry = data.CasterGUID.GetEntry();
        fields.casterLow = data.CasterGUID.GetLow();
        fields.mapId = data.CasterGUID.GetMapId();

        return ParseResult{ w.TakeString(), std::move(fields) };
    }
}

#endif