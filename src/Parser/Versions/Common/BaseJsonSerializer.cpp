#include "BaseJsonSerializer.h"
#include "Misc/Utilities.h"
#include "Misc/WowGuid.h"
#include <fmt/core.h>

using namespace PktParser::Reader;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;

namespace PktParser::Versions::Common
{
    json BaseJsonSerializer::SerializePacketHead(PktHeader const& header, char const* opcodeName, uint32 build) const
    {
        json j;
        j["Direction"] = Misc::DirectionToString(header.direction);
        j["PacketName"] = opcodeName;
        j["ConnectionIndex"] = header.connectionIndex;
        j["TickCount"] = header.tickCount;
        j["Timestamp"] = Misc::FormatUnixMilliseconds(header.timestamp);
        j["Opcode"] = fmt::format("0x{:06X}", header.opcode);
        j["Length"] = header.packetLength - 4;
        j["Build"] = build;
        return j;
    }

    json BaseJsonSerializer::SerializeFullPacket(PktHeader const& header, char const* opcodeName, 
        uint32 build, uint32 pktNumber, json const& packetData) const
    {
        json j;
        j["Number"] = pktNumber;
        j["Header"] = SerializePacketHead(header, opcodeName, build);
        j["Data"] = packetData;
        return j;
    }

    json BaseJsonSerializer::SerializeAuthChallenge(AuthChallengeData const* data)
    {
        json j;
        j["DosChallenge"] = json::array();
        for (int i = 0; i < 8; i++)
            j["DosChallenge"].push_back(data->DosChallenge[i]);

        std::string challengeHex;
        challengeHex.reserve(64);
        for (int i = 0; i < 32; i++)
            fmt::format_to(std::back_inserter(challengeHex), "{:02X}", data->Challenge[i]);
        j["Challenge"] = challengeHex;
        j["DosZeroBits"] = data->DosZeroBits;
        return j;
    }

    json BaseJsonSerializer::SerializeUpdateWorldState(WorldStateInfo const* info, bool hidden)
    {
        json j;
        j["WorldStateId"] = info->VariableID;
        j["Value"] = info->Value;
        j["Hidden"] = hidden;
        return j;
    }
}