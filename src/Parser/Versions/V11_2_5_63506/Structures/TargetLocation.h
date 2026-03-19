#pragma once

#include "Misc/Define.h"
#include "Misc/WowGuid.h"

namespace PktParser::V11_2_5_63506::Structures
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
