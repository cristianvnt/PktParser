#include "AuthSerializer.h"

namespace PktParser::V11_2_5_63506::Serializers
{
    void SerializeAuthChallenge(JsonWriter& w, AuthChallengeData const& data)
    {
        w.BeginObject();

        w.Key("DosChallenge");
        w.BeginArray();
        for (int i = 0; i < 8; i++)
            w.UInt(data.DosChallenge[i]);
        w.EndArray();

        std::string challengeHex;
        challengeHex.reserve(64);
        for (int i = 0; i < 32; i++)
            fmt::format_to(std::back_inserter(challengeHex), "{:02X}", data.Challenge[i]);
        
        w.WriteString("Challenge", challengeHex);
        w.WriteUInt("DosZeroBits", data.DosZeroBits);

        w.EndObject();
    }
}