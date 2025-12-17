#ifndef SPELL_HANDLERS_INL
#define SPELL_HANDLERS_INL

#include "Reader/BitReader.h"
#include "Structures/SpellGoData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"
#include "Misc/WowGuid.h"
#include <nlohmann/json.hpp>

namespace PktParser::Common::Parsers::SpellHandlers
{
    using BitReader = PktParser::Reader::BitReader;
    using json = nlohmann::json;

    inline Structures::TargetLocation ReadLocation(BitReader& reader)
    {
        Structures::TargetLocation loc;
        loc.Transport = Misc::ReadPackedGuid128(reader);
        loc.X = reader.ReadFloat();
        loc.Y = reader.ReadFloat();
        loc.Z = reader.ReadFloat();

        return loc;
    }

    template<typename TSerializer, typename ParseTargetDataFunc>
    inline json ParseSpellGoDefault(BitReader& reader, TSerializer* serializer, ParseTargetDataFunc parseTargetData)
    {
        reader.ReadUInt32(); // skip opcode

        Structures::SpellGoData data{};

        data.CasterGUID = Misc::ReadPackedGuid128(reader);
        data.CasterUnit = Misc::ReadPackedGuid128(reader);
        data.CastID = Misc::ReadPackedGuid128(reader);
        data.OriginalCastID = Misc::ReadPackedGuid128(reader);

        Structures::SpellCastFixedData const* basicInfo = reader.ReadChunk<Structures::SpellCastFixedData>();
        data.FixedData = *basicInfo;

        Structures::SpellHealPrediction const* healPtr = reader.ReadChunk<Structures::SpellHealPrediction>();
        data.HealPrediction = *healPtr;
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

        data.RemainingPower.resize(data.RemainingPowerCount);
        for (uint32 i = 0; i < data.RemainingPowerCount; ++i)
        {
            data.RemainingPower[i].Type = reader.ReadUInt8();
            data.RemainingPower[i].Cost = reader.ReadInt32();
        }

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

        return serializer->SerializeSpellGo(data);
    }
}

#endif