#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <ctime>
#include <fmt/core.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <cassandra.h>
#include <zstd.h>
#include <filesystem>
#include <vector>

#include "Define.h"
#include "Enums/Direction.h"
#include "Enums/TargetFlags.h"

namespace PktParser::Misc
{

	inline std::vector<std::filesystem::path> CollectPktFiles(std::string const& input)
	{
		namespace fs = std::filesystem;
		std::vector<fs::path> files;

		fs::path inputPath(input);

		if (input.ends_with(".pkt"))
		{
			if (fs::exists(inputPath))
				files.push_back(inputPath);
			return files;
		}

		if (!fs::is_directory(inputPath))
			return files;

		for (auto const& entry : fs::directory_iterator(inputPath))
			if (entry.is_regular_file() && entry.path().extension() == ".pkt")
				files.push_back(entry.path());

		std::sort(files.begin(), files.end());
    	return files;
	}

	inline CassUuid GenerateFileId(uint32 startTime, size_t fileSize)
    {
        std::hash<uint64> hasher;

        uint64 combined1 = (static_cast<uint64>(startTime) << 32) | static_cast<uint64>(fileSize & 0xFFFFFFFF);
        uint64 combined2 = (static_cast<uint64>(fileSize) << 32) | static_cast<uint64>(startTime);
        
        CassUuid uuid;
        uuid.time_and_version = hasher(combined1);
        uuid.clock_seq_and_node = hasher(combined2);

        uuid.time_and_version = (uuid.time_and_version & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        uuid.clock_seq_and_node = (uuid.clock_seq_and_node & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        return uuid;
    }
	
	inline std::vector<uint8> CompressJson(std::string const& json, int level = 6)
    {
        size_t maxSize = ZSTD_compressBound(json.size());
        std::vector<uint8> compressed(maxSize);

        size_t compressedSize = ZSTD_compress(compressed.data(), compressed.size(), json.data(), json.size(), level);

        if (ZSTD_isError(compressedSize))
        {
            LOG("ZSTD compression failed: {}", ZSTD_getErrorName(compressedSize));
            compressed.assign(json.begin(), json.end());
            return compressed;
        }

        compressed.resize(compressedSize);
        return compressed;
    }

	inline std::vector<uint8> CompressData(std::vector<uint8> const& input, int level = 6)
    {
        size_t maxSize = ZSTD_compressBound(input.size());
        std::vector<uint8> compressed(maxSize);

        size_t compressedSize = ZSTD_compress(compressed.data(), compressed.size(), input.data(), input.size(), level);

        if (ZSTD_isError(compressedSize))
        {
            LOG("ZSTD compression failed: {}", ZSTD_getErrorName(compressedSize));
            return input;
        }

        compressed.resize(compressedSize);
        return compressed;
    }

	inline std::string Base64Encode(uint8 const* data, size_t len)
	{
		BIO* b64 = BIO_new(BIO_f_base64());
		BIO* mem = BIO_new(BIO_s_mem());
		b64 = BIO_push(b64, mem);

		BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
		BIO_write(b64, data, static_cast<int>(len));
		BIO_flush(b64);

		BUF_MEM* buffPtr;
		BIO_get_mem_ptr(b64, &buffPtr);

		std::string result(buffPtr->data, buffPtr->length);
		BIO_free_all(b64);
		
		return result;
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
#ifndef _WIN32
		localtime_r(&tt, &tm);
#else
		localtime_s(&tm, &tt);
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
