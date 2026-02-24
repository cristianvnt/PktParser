#ifndef BASE_JSON_SERIALIZER_H
#define BASE_JSON_SERIALIZER_H

#include "JsonWriter.h"
#include "Structures/SpellCastData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Structures/Packed/WorldStateInfo.h"
#include "Reader/PktFileReader.h"
#include "Misc/Utilities.h"
#include <fmt/core.h>

namespace PktParser::Common
{
    class BaseJsonSerializer
    {
    public:
        virtual ~BaseJsonSerializer() = default;
        
        virtual void WriteSpellData(JsonWriter& w, Structures::SpellCastData const& data) const;
        virtual void WriteTargetData(JsonWriter& w, Structures::SpellTargetData const& target) const;
        
        // same everywhere (allegedly)
        static void WriteAuthChallenge(JsonWriter& w, Structures::Packed::AuthChallengeData const* data);
        static void WriteUpdateWorldState(JsonWriter& w, Structures::Packed::WorldStateInfo const* info, bool hidden);

        // helpers
        static void WriteGuidTargetFields(JsonWriter& w, Misc::WowGuid128 const& guid);
        static void WriteTargetLocation(JsonWriter& w, Structures::TargetLocation const& loc);

    protected:
        void WriteSpellDataFields(JsonWriter& w, Structures::SpellCastData const& data) const;
        void WriteTargetDataFields(JsonWriter& w, Structures::SpellTargetData const& target) const;
    };
}
#endif