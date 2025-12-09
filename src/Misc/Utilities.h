#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <ctime>
#include <chrono>
#include <fmt/core.h>

#include "Define.h"
#include "Parser/Direction.h"

namespace Utilities
{
	template<typename T>
	inline std::string FormatUnixMilliseconds(T value)
	{
		int64 totalMs;

		if constexpr (std::is_floating_point_v<T>)
			totalMs = static_cast<int64>(value);
		else
			totalMs = static_cast<int64>(value) * 1000;

		int64 sec = totalMs / 1000;
		int ms = totalMs % 1000;

		std::time_t tt = static_cast<std::time_t>(sec);
		std::tm tm;
		
#ifdef _MSC_VER
		localtime_s(&tm, &tt);
#else
		tm = *std::localtime(&tt);
#endif

		char buffer[64];
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);

		return fmt::format("{}.{:03d}", buffer, ms);
	}

	inline char const* DirectionToString(PktParser::Direction dir)
	{
		using PktParser::Direction;

		switch (dir)
		{
		case Direction::ClientToServer:
			return "CMSG";
		case Direction::ServerToClient:
			return "SMSG";
		case Direction::BNClientToServer:
			return "BN_CMSG";
		case Direction::BNServerToClient:
			return "BN_SMSG";
		case Direction::Bidirectional:
			return "MSG";
		default:
			return "UNKNOWN";
		}
	}
}

#endif // !UTILITIES_H
