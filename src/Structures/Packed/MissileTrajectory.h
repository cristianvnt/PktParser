#ifndef MISSILE_TRAJECTORY_H
#define MISSILE_TRAJECTORY_H

#include "Misc/Define.h"

namespace PktParser::Structures::Packed
{
#pragma pack(push, 1)
	struct MissileTrajectoryResult
	{
		uint32 TravelTime;
		float Pitch;
	};
#pragma pack(pop)
}

#endif