#include "SpellSearchFields.h"

using namespace PktParser::V11_2_5_63506::Structures;
using namespace PktParser::Common;

namespace PktParser::V11_2_5_63506::SearchFields
{
    SpellSearchFields FillSpellFields(SpellCastData const& data)
    {
        SpellSearchFields fields{};
        fields.spellId = data.FixedData.SpellID;
        fields.castId = data.CastID.ToHexString();
        fields.originalCastId = data.OriginalCastID.ToHexString();
        fields.casterGuid = data.CasterGUID.ToHexString();
        fields.casterType = Enums::GuidTypeToString(data.CasterGUID.GetType());
        fields.casterEntry = data.CasterGUID.GetEntry();
        fields.casterLow = data.CasterGUID.GetLow();
        fields.mapId = data.CasterGUID.GetMapId();

        for (auto const& guid : data.HitTargets)
            fields.hitTargetEntries.push_back(guid.GetEntry());

        return fields;
    }
}