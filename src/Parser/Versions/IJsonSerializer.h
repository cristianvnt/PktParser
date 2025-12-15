#ifndef IJSON_SERIALIZER_H
#define IJSON_SERIALIZER_H

#include "Reader/PktFileReader.h"
#include "Structures/SpellGoData.h"
#include "Structures/SpellTargetData.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "Structures/Packed/WorldStateInfo.h"
#include <nlohmann/json.hpp>

namespace PktParser::Versions
{
    using json = nlohmann::ordered_json;

    class IJsonSerializer
    {
    public:
        virtual ~IJsonSerializer() = default;

        virtual json SerializePacketHead(Reader::PktHeader const& header, char const* opcodeName, uint32 build) const = 0;
        virtual json SerializeFullPacket(Reader::PktHeader const& header, char const* opcodeName,
            uint32 build, uint32 pktNumber, json const& packetData) const = 0;
    };
}

#endif
