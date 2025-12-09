#include "Parser.h"
#include "Misc/Logger.h"
#include "Misc/WowGuid.h"
#include "Misc/Utilities.h"
#include "PktStructures.h"
#include "PktHandler.h"

#include <vector>
#include <fmt/core.h>
#include <chrono>

namespace PktParser
{
	void Parser::RegisterHandlers(PktRouter& router)
	{
		router.RegisterHandler(Opcode::SMSG_AUTH_CHALLENGE, ParseAuthChallenge);
		router.RegisterHandler(Opcode::SMSG_SPELL_GO, ParseSpellGo);
	}

	void Parser::ParseAuthChallenge(BitReader& reader)
	{
		LOG("===== Parsing SMSG_AUTH_CHALLANGE =====");
		reader.ReadUInt32(); // skip opcode

		for (size_t i = 0; i < 8; ++i)
		{
			uint32 dosChallenge = reader.ReadUInt32();
			LOG("	[{}] DosChallange: {}", i, dosChallenge);
		}

		std::vector<uint8> challenge(32);
		for (size_t i = 0; i < 32; ++i)
			challenge[i] = reader.ReadUInt8();

		std::string challengeHex;
		for (uint8 b : challenge)
			fmt::format_to(std::back_inserter(challengeHex), "{:02X}", b);
		LOG("Challenge: {}", challengeHex);

		uint8 dosZeroBits = reader.ReadUInt8();
		LOG("DosZeroBits: {}", dosZeroBits);

		LOG("===== AUTH_CHALLENGE Parsed =====");
		LOG("");
	}

	void Parser::ParseSpellGo(BitReader& reader)
	{
		reader.ReadUInt32(); // skip opcode
		reader.ResetBitReader();

		WowGuid128 casterGUID = ReadPackedGuid128(reader);
		WowGuid128 casterUnit = ReadPackedGuid128(reader);
		WowGuid128 castID = ReadPackedGuid128(reader);
		WowGuid128 originalCastID = ReadPackedGuid128(reader);

		SpellCastFixedData const* basicInfo = reader.ReadChunk<SpellCastFixedData>();
		SpellHealPrediction const* healPrediction = reader.ReadChunk<SpellHealPrediction>();

		WowGuid128 beaconGUID = ReadPackedGuid128(reader);

		uint16 hitTargetsCount = reader.ReadBits(16);
		uint16 missTargetsCount = reader.ReadBits(16);
		uint16 hitStatusCount = reader.ReadBits(16);
		uint16 missStatusCount = reader.ReadBits(16);
		uint16 remainingPowerCount = reader.ReadBits(9);
		bool hasRemainingRunes = reader.ReadBit();
		uint16 targetPointsCount = reader.ReadBits(16);

		reader.ResetBitReader();

		std::vector<WowGuid128> hitTargets;
		hitTargets.reserve(hitTargetsCount);
		for (uint16 i = 0; i < hitTargetsCount; ++i)
			hitTargets.push_back(ReadPackedGuid128(reader));

		std::vector<WowGuid128> missTargets;
		missTargets.reserve(missTargetsCount);
		for (uint16 i = 0; i < missTargetsCount; ++i)
			missTargets.push_back(ReadPackedGuid128(reader));

		std::vector<SpellHitStatus> hitStatus;
		hitStatus.reserve(hitStatusCount);
		for (uint16 i = 0; i < hitStatusCount; ++i)
		{
			SpellHitStatus status;
			status.Target = ReadPackedGuid128(reader);
			status.HitResult = reader.ReadUInt8();
			hitStatus.push_back(status);
		}

		std::vector<SpellMissStatus> missStatus;
		missStatus.reserve(missStatusCount);
		for (uint16 i = 0; i < missStatusCount; ++i)
		{
			SpellMissStatus status;
			status.Target = ReadPackedGuid128(reader);
			status.MissReason = reader.ReadUInt8();
			status.ReflectStatus = reader.ReadUInt8();
			missStatus.push_back(status);
		}

		std::vector<uint32> remainingPower;
		reader.ReadChunkArray(remainingPower, remainingPowerCount);

		if (hasRemainingRunes)
		{
			RuneData const* runeData = reader.ReadChunk<RuneData>();
			uint32 cooldownsCount = reader.ReadUInt32();

			std::vector<uint8> cooldowns;
			reader.ReadChunkArray(cooldowns, cooldownsCount);
		}

		std::vector<TargetLocation> targetPoints;
		targetPoints.reserve(targetPointsCount);
		for (uint16 i = 0; i < targetPointsCount; ++i)
		{
			TargetLocation loc;
			loc.Transport = ReadPackedGuid128(reader);
			TargetLocationData const* locData = reader.ReadChunk<TargetLocationData>();
			loc.X = locData->X;
			loc.Y = locData->Y;
			loc.Z = locData->Z;
			targetPoints.push_back(loc);
		}
	}

}