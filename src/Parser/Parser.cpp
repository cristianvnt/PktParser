#include "Parser.h"
#include "Misc/Logger.h"
#include "Misc/WowGuid.h"
#include "Misc/Utilities.h"
#include "PacketStructures.h"

#include <vector>
#include <fmt/core.h>
#include <chrono>

namespace PktParser
{
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
		LOG("===== Parsing SMSG_SPELL_GO (SEQ) =====");

		auto start = std::chrono::high_resolution_clock::now();

		reader.ReadUInt32(); // skip opcode
		reader.ResetBitReader();
		WowGuid128 casterGUID = ReadPackedGuid128(reader);
		LOG("CasterGUID: Full: 0x{:016X}{:016X} Item/{} R{}/S{} Map: {} Low: {}", casterGUID.High, casterGUID.Low,
			casterGUID.GetSubType(), casterGUID.GetRealmId(), casterGUID.GetServerId(), casterGUID.GetMapId(), casterGUID.GetLow());

		WowGuid128 casterUnit = ReadPackedGuid128(reader);
		LOG("CasterUnit: 0x{:016X}{:016X}", casterUnit.High, casterUnit.Low);

		WowGuid128 castID = ReadPackedGuid128(reader);
		LOG("CastID: 0x{:016X}{:016X}", castID.High, castID.Low);

		WowGuid128 originalCastID = ReadPackedGuid128(reader);
		LOG("OriginalCastID: 0x{:016X}{:016X}", originalCastID.High, originalCastID.Low);

		int32 spellID = reader.ReadInt32();
		LOG("SpellID: {}", spellID);

		int32 spellXSpellVisualID = reader.ReadInt32();
		int32 scriptVisualID = reader.ReadInt32();
		LOG("Visual: {} / {}", spellXSpellVisualID, scriptVisualID);

		uint32 castFlags = reader.ReadUInt32();
		uint32 castFlagsEx = reader.ReadUInt32();
		uint32 castFlagsEx2 = reader.ReadUInt32();
		uint32 castTime = reader.ReadUInt32();
		LOG("CastFlags: 0x{:X}, Ex: 0x{:X}, Ex2: 0x{:X}, Time: {}ms",
			castFlags, castFlagsEx, castFlagsEx2, Utilities::FormatUnixMilliseconds(castTime));

		uint32 travelTime = reader.ReadUInt32();
		float pitch = reader.ReadFloat();
		LOG("Missile: TravelTime={}ms, Pitch={}", travelTime, pitch);

		int32 ammoDisplayID = reader.ReadInt32();
		uint8 destLocSpellCastIndex = reader.ReadUInt8();
		LOG("AmmoDisplayID: {}, DestLocIndex: {}", ammoDisplayID, destLocSpellCastIndex);

		int32 immunitySchool = reader.ReadInt32();
		int32 immunityValue = reader.ReadInt32();
		LOG("Immunities: School={}, Value={}", immunitySchool, immunityValue);

		int32 healPoints = reader.ReadInt32();
		uint8 healType = reader.ReadUInt8();
		WowGuid128 beaconGUID = ReadPackedGuid128(reader);
		LOG("HealPrediction: Points={}, Type={}, BeaconGUID=0x{:016X}{:016X}",
			healPoints, healType, beaconGUID.High, beaconGUID.Low);

		uint16 hitTargetsCount = reader.ReadBits(16);
		uint16 missTargetsCount = reader.ReadBits(16);
		uint16 hitStatusCount = reader.ReadBits(16);
		uint16 missStatusCount = reader.ReadBits(16);
		uint16 remainingPowerCount = reader.ReadBits(9);
		bool hasRemainingRunes = reader.ReadBit();
		uint16 targetPointsCount = reader.ReadBits(16);
		LOG("Counts: Hit={}, Miss={}, HitStatus={}, MissStatus={}, Power={}, HasRunes={}, Points={}",
			hitTargetsCount, missTargetsCount, hitStatusCount, missStatusCount,
			remainingPowerCount, hasRemainingRunes, targetPointsCount);

		reader.ResetBitReader();

		std::vector<WowGuid128> hitTargets;
		hitTargets.reserve(hitTargetsCount);
		for (uint16 i = 0; i < hitTargetsCount; i++)
		{
			WowGuid128 target = ReadPackedGuid128(reader);
			hitTargets.push_back(target);
			LOG("  HitTarget[{}]: 0x{:016X}{:016X}", i, target.High, target.Low);
		}

		std::vector<SpellHitStatus> hitStatus;
		hitStatus.reserve(hitStatusCount);
		for (uint16 i = 0; i < hitStatusCount; i++)
		{
			SpellHitStatus status;
			status.Target = ReadPackedGuid128(reader);
			status.HitResult = reader.ReadUInt8();
			hitStatus.push_back(status);
			LOG("  HitStatus[{}]: Target=0x{:016X}{:016X}, Result={}",
				i, status.Target.High, status.Target.Low, status.HitResult);
		}

		std::vector<SpellMissStatus> missStatus;
		missStatus.reserve(missStatusCount);
		for (uint16 i = 0; i < missStatusCount; i++)
		{
			SpellMissStatus status;
			status.Target = ReadPackedGuid128(reader);
			status.MissReason = reader.ReadUInt8();
			status.ReflectStatus = reader.ReadUInt8();
			missStatus.push_back(status);
			LOG("  MissStatus[{}]: Target=0x{:016X}{:016X}, Reason={}, Reflect={}",
				i, status.Target.High, status.Target.Low,
				status.MissReason, status.ReflectStatus);
		}

		std::vector<uint32> remainingPower;
		remainingPower.reserve(remainingPowerCount);
		for (uint16 i = 0; i < remainingPowerCount; i++)
		{
			uint32 power = reader.ReadUInt32();
			remainingPower.push_back(power);
			LOG("  RemainingPower[{}]: {}", i, power);
		}

		if (hasRemainingRunes)
		{
			uint8 runeStart = reader.ReadUInt8();
			uint8 runeCount = reader.ReadUInt8();
			uint32 cooldownsCount = reader.ReadUInt32();

			LOG("RuneData: Start={}, Count={}, Cooldowns={}",
				runeStart, runeCount, cooldownsCount);

			std::vector<uint8> cooldowns;
			cooldowns.reserve(cooldownsCount);
			for (uint32 i = 0; i < cooldownsCount; i++)
			{
				uint8 cooldown = reader.ReadUInt8();
				cooldowns.push_back(cooldown);
				LOG("    Cooldown[{}]: {}", i, cooldown);
			}
		}

		std::vector<TargetLocation> targetPoints;
		targetPoints.reserve(targetPointsCount);
		for (uint16 i = 0; i < targetPointsCount; i++)
		{
			TargetLocation loc;
			loc.Transport = ReadPackedGuid128(reader);
			loc.X = reader.ReadFloat();
			loc.Y = reader.ReadFloat();
			loc.Z = reader.ReadFloat();
			targetPoints.push_back(loc);
			LOG("  TargetPoint[{}]: Transport=0x{:016X}{:016X}, Pos=({}, {}, {})",
				i, loc.Transport.High, loc.Transport.Low, loc.X, loc.Y, loc.Z);
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

		LOG("Sequential parse completed in {}ns", duration.count());
		LOG("===== SMSG_SPELL_GO Parsed =====");
		LOG("");
	}

	void Parser::ParseSpellGo_CHUNKED(BitReader& reader)
	{
		LOG("===== Parsing SMSG_SPELL_GO (CHUNKED) =====");

		auto start = std::chrono::high_resolution_clock::now();

		reader.ReadUInt32(); // skip opcode
		reader.ResetBitReader();

		WowGuid128 casterGUID = ReadPackedGuid128(reader);
		LOG("CasterGUID: Full: 0x{:016X}{:016X} Item/{} R{}/S{} Map: {} Low: {}", casterGUID.High, casterGUID.Low,
			casterGUID.GetSubType(), casterGUID.GetRealmId(), casterGUID.GetServerId(), casterGUID.GetMapId(), casterGUID.GetLow());

		WowGuid128 casterUnit = ReadPackedGuid128(reader);
		LOG("CasterUnit: 0x{:016X}{:016X}", casterUnit.High, casterUnit.Low);

		WowGuid128 castID = ReadPackedGuid128(reader);
		LOG("CastID: 0x{:016X}{:016X}", castID.High, castID.Low);

		WowGuid128 originalCastID = ReadPackedGuid128(reader);
		LOG("OriginalCastID: 0x{:016X}{:016X}", originalCastID.High, originalCastID.Low);

		// 49 bytes chunk
		SpellCastFixedData const* basicInfo = reinterpret_cast<SpellCastFixedData const*>(reader.GetCurrentPtr());
		reader.Skip(sizeof(SpellCastFixedData));

		LOG("SpellID: {}", basicInfo->SpellID);
		LOG("Visual: {} / {}", basicInfo->Visual.SpellXSpellVisualID, basicInfo->Visual.ScriptVisualID);
		LOG("CastFlags: 0x{:X}, Ex: 0x{:X}, Ex2: 0x{:X}, Time: {}ms",
			basicInfo->CastFlags, basicInfo->CastFlagsEx, basicInfo->CastFlagsEx2, Utilities::FormatUnixMilliseconds(basicInfo->CastTime));
		LOG("Missile: TravelTime={}ms, Pitch={}", basicInfo->MissileTrajectory.TravelTime, basicInfo->MissileTrajectory.Pitch);
		LOG("AmmoDisplayID: {}, DestLocIndex: {}", basicInfo->AmmoDisplayID, basicInfo->DestLocSpellCastIndex);
		LOG("Immunities: School={}, Value={}", basicInfo->Immunities.School, basicInfo->Immunities.Value);

		// normal
		int32 healPoints = reader.ReadInt32();
		uint8 healType = reader.ReadUInt8();
		WowGuid128 beaconGUID = ReadPackedGuid128(reader);
		LOG("HealPrediction: Points={}, Type={}, BeaconGUID=0x{:016X}{:016X}",
			healPoints, healType, beaconGUID.High, beaconGUID.Low);

		uint16 hitTargetsCount = reader.ReadBits(16);
		uint16 missTargetsCount = reader.ReadBits(16);
		uint16 hitStatusCount = reader.ReadBits(16);
		uint16 missStatusCount = reader.ReadBits(16);
		uint16 remainingPowerCount = reader.ReadBits(9);
		bool hasRemainingRunes = reader.ReadBit();
		uint16 targetPointsCount = reader.ReadBits(16);
		LOG("Counts: Hit={}, Miss={}, HitStatus={}, MissStatus={}, Power={}, HasRunes={}, Points={}",
			hitTargetsCount, missTargetsCount, hitStatusCount, missStatusCount,
			remainingPowerCount, hasRemainingRunes, targetPointsCount);

		reader.ResetBitReader();

		std::vector<WowGuid128> hitTargets;
		hitTargets.reserve(hitTargetsCount);
		for (uint16 i = 0; i < hitTargetsCount; i++)
		{
			WowGuid128 target = ReadPackedGuid128(reader);
			hitTargets.push_back(target);
			LOG("  HitTarget[{}]: 0x{:016X}{:016X}", i, target.High, target.Low);
		}

		std::vector<SpellHitStatus> hitStatus;
		hitStatus.reserve(hitStatusCount);
		for (uint16 i = 0; i < hitStatusCount; i++)
		{
			SpellHitStatus status;
			status.Target = ReadPackedGuid128(reader);
			status.HitResult = reader.ReadUInt8();
			hitStatus.push_back(status);
			LOG("  HitStatus[{}]: Target=0x{:016X}{:016X}, Result={}",
				i, status.Target.High, status.Target.Low, status.HitResult);
		}

		std::vector<SpellMissStatus> missStatus;
		missStatus.reserve(missStatusCount);
		for (uint16 i = 0; i < missStatusCount; i++)
		{
			SpellMissStatus status;
			status.Target = ReadPackedGuid128(reader);
			status.MissReason = reader.ReadUInt8();
			status.ReflectStatus = reader.ReadUInt8();
			missStatus.push_back(status);
			LOG("  MissStatus[{}]: Target=0x{:016X}{:016X}, Reason={}, Reflect={}",
				i, status.Target.High, status.Target.Low,
				status.MissReason, status.ReflectStatus);
		}

		std::vector<uint32> remainingPower;
		remainingPower.reserve(remainingPowerCount);
		for (uint16 i = 0; i < remainingPowerCount; i++)
		{
			uint32 power = reader.ReadUInt32();
			remainingPower.push_back(power);
			LOG("  RemainingPower[{}]: {}", i, power);
		}

		if (hasRemainingRunes)
		{
			uint8 runeStart = reader.ReadUInt8();
			uint8 runeCount = reader.ReadUInt8();
			uint32 cooldownsCount = reader.ReadUInt32();

			LOG("RuneData: Start={}, Count={}, Cooldowns={}",
				runeStart, runeCount, cooldownsCount);

			std::vector<uint8> cooldowns;
			cooldowns.reserve(cooldownsCount);
			for (uint32 i = 0; i < cooldownsCount; i++)
			{
				uint8 cooldown = reader.ReadUInt8();
				cooldowns.push_back(cooldown);
				LOG("    Cooldown[{}]: {}", i, cooldown);
			}
		}

		std::vector<TargetLocation> targetPoints;
		targetPoints.reserve(targetPointsCount);
		for (uint16 i = 0; i < targetPointsCount; i++)
		{
			TargetLocation loc;
			loc.Transport = ReadPackedGuid128(reader);
			loc.X = reader.ReadFloat();
			loc.Y = reader.ReadFloat();
			loc.Z = reader.ReadFloat();
			targetPoints.push_back(loc);
			LOG("  TargetPoint[{}]: Transport=0x{:016X}{:016X}, Pos=({}, {}, {})",
				i, loc.Transport.High, loc.Transport.Low, loc.X, loc.Y, loc.Z);
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

		LOG("CHUNKED parse completed in {}ns", duration.count());
		LOG("===== SMSG_SPELL_GO Parsed =====");
		LOG("");
	}

}