#ifndef AUTH_CHALLENGE_DATA_H
#define AUTH_CHALLENGE_DATA_H

#include "Misc/Define.h"

namespace PktParser::Structures::Packed
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

#endif // !AUTH_CHALLENGE_DATA_H