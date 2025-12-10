#include "Parser.h"
#include "Misc/Logger.h"
#include "Misc/WowGuid.h"
#include "Misc/Utilities.h"
#include "PktHandler.h"
#include "JsonSerializer.h"
#include "Enums/TargetFlags.h"
#include "Enums/CastFlags.h"
#include "Enums/SpellMissType.h"

#include <vector>
#include <fmt/core.h>
#include <chrono>

using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Enums;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;

namespace PktParser
{
	void Parser::RegisterHandlers(PktRouter& router)
	{
		router.RegisterHandler(Opcode::SMSG_AUTH_CHALLENGE, ParseAuthChallenge);
		router.RegisterHandler(Opcode::SMSG_SPELL_GO, ParseSpellGo);
		router.RegisterHandler(Opcode::SMSG_UPDATE_WORLD_STATE, ParseUpdateWorldState);
	}

	json Parser::ParseAuthChallenge(BitReader& reader, uint32 pktNumber)
	{
		reader.ReadUInt32(); // skip opcode
		reader.ResetBitReader();
		AuthChallengeData const* authData = reader.ReadChunk<AuthChallengeData>();

		return JsonSerializer::SerializeAuthChallenge(authData);
	}

	TargetLocation ReadLocation(BitReader& reader)
	{
		TargetLocation loc;
		loc.Transport = ReadPackedGuid128(reader);
		loc.X = reader.ReadFloat();
		loc.Y = reader.ReadFloat();
		loc.Z = reader.ReadFloat();
		return loc;
	}

	json Parser::ParseSpellGo(BitReader& reader, uint32 pktNumber)
	{
		reader.ReadUInt32(); // skip opcode

		SpellGoData data{};

		data.CasterGUID = ReadPackedGuid128(reader);
		data.CasterUnit = ReadPackedGuid128(reader);
		data.CastID = ReadPackedGuid128(reader);
		data.OriginalCastID = ReadPackedGuid128(reader);

		SpellCastFixedData const* basicInfo = reader.ReadChunk<SpellCastFixedData>();
		data.FixedData = *basicInfo;

		SpellHealPrediction const* healPtr = reader.ReadChunk<SpellHealPrediction>();
		data.HealPrediction = *healPtr;
		data.BeaconGUID = ReadPackedGuid128(reader);

		reader.ResetBitReader();

		data.HitTargetsCount = reader.ReadBits(16);
		data.MissTargetsCount = reader.ReadBits(16);
		data.HitStatusCount = reader.ReadBits(16);
		data.MissStatusCount = reader.ReadBits(16);
		data.RemainingPowerCount = reader.ReadBits(9);
		data.HasRuneData = reader.ReadBit();
		data.TargetPointsCount = reader.ReadBits(16);

		data.MissStatus.reserve(data.MissStatusCount);
		for (uint32 i = 0; i < data.MissStatusCount; ++i)
		{
			SpellMissStatus status;
			status.MissReason = reader.ReadUInt8();
			if (status.MissReason == 11) // SPELL_MISS_REFLECT
				status.ReflectStatus = reader.ReadUInt8();
			else
				status.ReflectStatus = 0;
			data.MissStatus.push_back(status);
		}

		ParseSpellTargetData(reader, basicInfo->SpellID, data.TargetData);

		data.HitTargets.reserve(data.HitTargetsCount);
		for (uint32 i = 0; i < data.HitTargetsCount; ++i)
			data.HitTargets.push_back(ReadPackedGuid128(reader));

		data.MissTargets.reserve(data.MissTargetsCount);
		for (uint32 i = 0; i < data.MissTargetsCount; ++i)
			data.MissTargets.push_back(ReadPackedGuid128(reader));

		data.HitStatus.reserve(data.HitStatusCount);
		if (data.HitStatusCount > 0)
			reader.ReadChunkArray(data.HitStatus, data.HitStatusCount);

		data.RemainingPower.reserve(data.RemainingPowerCount);
		if (data.RemainingPowerCount > 0)
			reader.ReadChunkArray(data.RemainingPower, data.RemainingPowerCount);

		if (data.HasRuneData)
		{
			RuneData const* runePtr = reader.ReadChunk<RuneData>();
			data.Runes = *runePtr;

			data.RuneCooldowns.reserve(data.Runes.Count);
			if (data.Runes.Count > 0)
				reader.ReadChunkArray(data.RuneCooldowns, data.Runes.Count);
		}

		data.TargetPoints.reserve(data.TargetPointsCount);
		for (uint32 i = 0; i < data.TargetPointsCount; ++i)
			data.TargetPoints.push_back(ReadLocation(reader));

		return JsonSerializer::SerializeSpellGo(data);
	}

	json Parser::ParseUpdateWorldState(BitReader& reader, uint32 pktNumber)
	{
		reader.ReadUInt32(); // skip opcode

		WorldStateInfo const* worldStateInfo = reader.ReadChunk<WorldStateInfo>();

		reader.ResetBitReader();
		bool hidden = reader.ReadBit();

		return JsonSerializer::SerializeUpdateWorldState(worldStateInfo, hidden);
	}

	void Parser::ParseSpellTargetData(BitReader& reader, uint32 spellID, SpellTargetData& targetData)
	{
		reader.ResetBitReader();

		targetData.Flags = reader.ReadUInt32();
		targetData.Unit = ReadPackedGuid128(reader);
		targetData.Item = ReadPackedGuid128(reader);

		targetData.HasSrcLocation = reader.ReadBit();
		targetData.HasDstLocation = reader.ReadBit();
		targetData.HasOrientation = reader.ReadBit();
		targetData.HasMapID = reader.ReadBit();

		uint32 nameLength = reader.ReadBits(7);

		if (targetData.HasSrcLocation)
			targetData.SrcLocation = ReadLocation(reader);

		if (targetData.HasDstLocation)
			targetData.DstLocation = ReadLocation(reader);

		if (targetData.HasOrientation)
			targetData.Orientation = reader.ReadFloat();

		if (targetData.HasMapID)
			targetData.MapID = reader.ReadUInt32();

		targetData.Name = reader.ReadWoWString(nameLength);
	}
}