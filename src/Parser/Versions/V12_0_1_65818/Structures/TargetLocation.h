#pragma once

#include "Misc/Define.h"
#include "Misc/WowGuid.h"

namespace PktParser::V12_0_1_65818::Structures
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
