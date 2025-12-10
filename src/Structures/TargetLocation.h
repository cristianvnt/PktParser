#ifndef TARGET_LOCATION_H
#define TARGET_LOCATION_H

#include "Misc/Define.h"
#include "Misc/WowGuid.h"

namespace PktParser::Structures
{
	using WowGuid128 = PktParser::Misc::WowGuid128;

	struct TargetLocation
	{
		WowGuid128 Transport;
		float X;
		float Y;
		float Z;
	};
}

#endif // !TARGET_LOCATION_H