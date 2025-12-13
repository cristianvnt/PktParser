#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <ctime>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <fmt/core.h>

#include "Define.h"
#include "Enums/Direction.h"
#include "Enums/TargetFlags.h"
#include "Logger.h"

namespace PktParser::Misc
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

	inline char const* DirectionToString(PktParser::Enums::Direction dir)
	{
		using PktParser::Enums::Direction;

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

	inline std::string GetTargetFlagName(uint32 flags)
	{
		using PktParser::Enums::TargetFlag;

		if (flags == TargetFlag::Self)
			return "Self (0x00000000)";

		std::string result = fmt::format("0x{:08X} (", flags);
		bool first = true;

		auto addFlag = [&](TargetFlag flag, const char* name)
		{
			if (flags & flag)
			{
				if (!first)
					result += " | ";
				result += name;
				first = false;
			}
		};

		addFlag(TargetFlag::SpellDynamic1, "SpellDynamic1");
		addFlag(TargetFlag::Unit, "Unit");
		addFlag(TargetFlag::UnitRaid, "UnitRaid");
		addFlag(TargetFlag::UnitParty, "UnitParty");
		addFlag(TargetFlag::Item, "Item");
		addFlag(TargetFlag::SourceLocation, "SourceLocation");
		addFlag(TargetFlag::DestinationLocation, "DestinationLocation");
		addFlag(TargetFlag::UnitEnemy, "UnitEnemy");
		addFlag(TargetFlag::UnitAlly, "UnitAlly");
		addFlag(TargetFlag::CorpseEnemy, "CorpseEnemy");
		addFlag(TargetFlag::UnitDead, "UnitDead");
		addFlag(TargetFlag::GameObject, "GameObject");
		addFlag(TargetFlag::TradeItem, "TradeItem");
		addFlag(TargetFlag::NameString, "NameString");
		addFlag(TargetFlag::GameObjectItem, "GameObjectItem");
		addFlag(TargetFlag::CorpseAlly, "CorpseAlly");
		addFlag(TargetFlag::UnitMinipet, "UnitMinipet");
		addFlag(TargetFlag::Glyph, "Glyph");
		addFlag(TargetFlag::DestinationTarget, "DestinationTarget");
		addFlag(TargetFlag::ExtraTargets, "ExtraTargets");
		addFlag(TargetFlag::UnitPassenger, "UnitPassenger");
		addFlag(TargetFlag::Unk400000, "Unk400000");
		addFlag(TargetFlag::Unk1000000, "Unk1000000");
		addFlag(TargetFlag::Unk4000000, "Unk4000000");
		addFlag(TargetFlag::Unk10000000, "Unk10000000");
		addFlag(TargetFlag::Unk40000000, "Unk40000000");

		result += first ? "None)" : ")";
		return result;
	}
}

#endif // !UTILITIES_H
