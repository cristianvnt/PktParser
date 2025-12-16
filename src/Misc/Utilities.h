#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <ctime>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <fmt/core.h>
#include <array>

#include "Define.h"
#include "Enums/Direction.h"
#include "Enums/TargetFlags.h"
#include "Logger.h"
#include "Database/Database.h"

namespace PktParser::Misc
{
	inline void SeedBuildMappings(PktParser::Db::Database& db)
	{
		constexpr std::array<uint32, 12> builds1125 = { 63506, 63660, 63704, 63796, 63825, 63834, 63906, 64154, 64270, 64395, 64484, 64502 };
		for (uint32 build : builds1125)
			db.InsertBuildMapping({ build, "11.2.5", 11, 2, 5, "V11_2_5_64502" });

    	constexpr std::array<uint32, 6> builds1127 = { 64632, 64704, 64725, 64743, 64772, 64797 };
		for (uint32 build : builds1127)
			db.InsertBuildMapping({ build, "11.2.7", 11, 2, 7, "V11_2_7_64632" });
	}

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
