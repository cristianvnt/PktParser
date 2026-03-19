#pragma once
#include "Misc/Define.h"

namespace PktParser::V11_2_0_62213::Structures
{
#pragma pack(push, 1)
	struct AuthChallengeData
	{
		uint32 DosChallenge[8];
		uint8 Challenge[32];
		uint8 DosZeroBits;
	};
#pragma pack(pop)
}
