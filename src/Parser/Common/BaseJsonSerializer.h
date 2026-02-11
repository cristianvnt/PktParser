#ifndef BASE_JSON_SERIALIZER_H
#define BASE_JSON_SERIALIZER_H

#include "Structures/SpellCastData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/TargetLocation.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Structures/Packed/WorldStateInfo.h"
#include "Reader/PktFileReader.h"
#include "Misc/Utilities.h"
#include <fmt/core.h>
#include <nlohmann/json.hpp>

namespace PktParser::Common
{
    using json = nlohmann::ordered_json;

    class BaseJsonSerializer
    {
    public:
        virtual ~BaseJsonSerializer() = default;
        
        json SerializeFullPacket(Reader::PktHeader const& header, char const* opcodeName,
            uint32 build, uint32 pktNumber, json&& pktData) const;
        json SerializePacketHead(Reader::PktHeader const& header, char const* opcodeName, uint32 build) const;

        // same everywhere (allegedly)
        static json SerializeAuthChallenge(Structures::Packed::AuthChallengeData const* data);
        static json SerializeUpdateWorldState(Structures::Packed::WorldStateInfo const* info, bool hidden);

        // commons
        virtual json SerializeSpellData(Structures::SpellCastData const& data) const;
        virtual json SerializeTargetData(Structures::SpellTargetData const& target) const;

        // helpers
        static json SerializeGuidTarget(Misc::WowGuid128 const& guid);
        static json SerializeTargetLocation(Structures::TargetLocation const& loc);
    };
}
#endif