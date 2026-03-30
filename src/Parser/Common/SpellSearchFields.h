#pragma once

#include "Misc/Define.h"
#include "JsonWriter.h"
#include "ISearchFields.h"

#include <string>
#include <vector>

namespace PktParser::Common
{
    struct SpellSearchFields final : ISearchFields
    {
        int32 spellId;
        std::string castId;
        std::string originalCastId;
        std::string casterGuid;
        std::string casterType;
        uint32 casterEntry;
        uint64 casterLow;
        int32 mapId;
        std::vector<uint32> hitTargetEntries;

        void WriteTo(JsonWriter& doc) const override
        {
            doc.WriteInt("spell_id", spellId);
            doc.WriteString("cast_id", castId);
            doc.WriteString("original_cast_id", originalCastId);
            doc.WriteString("caster_guid", casterGuid);
            doc.WriteString("caster_type", casterType);
            doc.WriteUInt("caster_entry", casterEntry);
            doc.WriteUInt("caster_low", casterLow);
            doc.WriteInt("map_id", mapId);

            if (!hitTargetEntries.empty())
            {
                doc.Key("hit_target_entries");
                doc.BeginArray();
                for (uint32 entry : hitTargetEntries)
                    doc.UInt(entry);
                doc.EndArray();
            }
        }
    };
}