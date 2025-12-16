#ifndef BASE_JSON_SERIALIZER_H
#define BASE_JSON_SERIALIZER_H

#include "IJsonSerializer.h"
#include "Misc/Utilities.h"
#include <fmt/core.h>

namespace PktParser::Versions::Common
{
    class BaseJsonSerializer : public IJsonSerializer
    {
    public:
        json SerializeFullPacket(Reader::PktHeader const& header, char const* opcodeName,
            uint32 build, uint32 pktNumber, json const& packetData) const override;
        json SerializePacketHead(Reader::PktHeader const& header, char const* opcodeName, uint32 build) const override;

        static json SerializeAuthChallenge(Structures::Packed::AuthChallengeData const* data);
        static json SerializeUpdateWorldState(Structures::Packed::WorldStateInfo const* info, bool hidden);

        // helpers
        static json SerializeGuidTarget(Misc::WowGuid128 const& guid);
        static json SerializeTargetLocation(Structures::TargetLocation const& loc);
    };
}
#endif