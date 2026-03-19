#pragma once

#include "Misc/Define.h"
#include "Misc/WowGuid.h"

namespace PktParser::V12_0_0_65390::Structures
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
