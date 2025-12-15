#include "Parser.h"
#include "Opcodes.h"
#include "Structures/Packed/AuthChallengeData.h"
#include "JsonSerializer.h"

using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Enums;
using namespace PktParser::Structures;
using namespace PktParser::Structures::Packed;

namespace PktParser::Versions::V11_2_7_64632
{
	ParserMethod Parser::GetParserMethod(uint32 opcode) const
	{
		switch(opcode)
		{
		case Opcodes::SMSG_AUTH_CHALLENGE:
			return &ParseAuthChallenge;	
		case Opcodes::SMSG_SPELL_GO:
			return &ParseSpellGo;
		case Opcodes::SMSG_UPDATE_WORLD_STATE:
			return &ParseUpdateWorldState;
		case Opcodes::CMSG_AUTH_SESSION:
		case Opcodes::SMSG_SPELL_START:
			return nullptr;
		default:
			return nullptr;
		}
	}

	char const* Parser::GetOpcodeName(uint32 opcode) const
	{
		auto it = OpcodeNames.find(opcode);
		return it != OpcodeNames.end() ? it->second : "UNKNOWN_OPCODE";
	}

	json Parser::ParseAuthChallenge(BitReader& reader, [[maybe_unused]] uint32 pktNumber)
	{
		reader.ReadUInt32(); // skip opcode
		reader.ResetBitReader();
		AuthChallengeData const* authData = reader.ReadChunk<AuthChallengeData>();

		return JsonSerializer::SerializeAuthChallenge(authData);
	}

	json Parser::ParseSpellGo(BitReader& reader, [[maybe_unused]] uint32 pktNumber)
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

		data.HitTargetsCount = reader.ReadBits(16);
		data.MissTargetsCount = reader.ReadBits(16);
		data.HitStatusCount = reader.ReadBits(16);
		data.MissStatusCount = reader.ReadBits(16);
		data.RemainingPowerCount = reader.ReadBits(9);
		data.HasRuneData = reader.ReadBit();
		data.TargetPointsCount = reader.ReadBits(16);

		reader.ResetBitReader();

		ParseSpellTargetData(reader, basicInfo->SpellID, data.TargetData);

		data.HitTargets.resize(data.HitTargetsCount);
		for (uint32 i = 0; i < data.HitTargetsCount; ++i)
			data.HitTargets[i] = ReadPackedGuid128(reader);

		data.MissTargets.resize(data.MissTargetsCount);
		for (uint32 i = 0; i < data.MissTargetsCount; ++i)
			data.MissTargets[i] = ReadPackedGuid128(reader);

		reader.ReadChunkArray(data.HitStatus, data.HitStatusCount);
		data.MissStatus.resize(data.MissStatusCount);
		for (uint32 i = 0; i < data.MissStatusCount; ++i)
		{
			data.MissStatus[i].MissReason = reader.ReadUInt8();
			if (data.MissStatus[i].MissReason == 11) // SPELL_MISS_REFLECT
				data.MissStatus[i].ReflectStatus = reader.ReadUInt8();
			else
				data.MissStatus[i].ReflectStatus = 0;
		}

		data.RemainingPower.resize(data.RemainingPowerCount);
		for (uint32 i = 0; i < data.RemainingPowerCount; ++i)
		{
			data.RemainingPower[i].Type = reader.ReadUInt8();
			data.RemainingPower[i].Cost = reader.ReadInt32();
		}

		if (data.HasRuneData)
		{
			RuneData const* runePtr = reader.ReadChunk<RuneData>();
			data.Runes = *runePtr;
			uint32 cooldownCount = reader.ReadUInt32();
			
			reader.ReadChunkArray(data.RuneCooldowns, cooldownCount);
		}

		data.TargetPoints.resize(data.TargetPointsCount);
		for (uint32 i = 0; i < data.TargetPointsCount; ++i)
			data.TargetPoints[i] = ReadLocation(reader);

		return JsonSerializer::SerializeSpellGo(data);
	}

	json Parser::ParseUpdateWorldState(BitReader& reader, [[maybe_unused]] uint32 pktNumber)
	{
		reader.ReadUInt32(); // skip opcode

		WorldStateInfo const* worldStateInfo = reader.ReadChunk<WorldStateInfo>();

		reader.ResetBitReader();
		bool hidden = reader.ReadBit();

		return JsonSerializer::SerializeUpdateWorldState(worldStateInfo, hidden);
	}

	void Parser::ParseSpellTargetData(BitReader& reader, [[maybe_unused]] uint32 spellID, SpellTargetData& targetData)
	{
		targetData.Flags = reader.ReadUInt32();
		targetData.Unit = ReadPackedGuid128(reader);
		targetData.Item = ReadPackedGuid128(reader);
		targetData.Unknown1127_1 = ReadPackedGuid128(reader);
		targetData.Unknown1127_2 = reader.ReadBit();

		bool hasSrc = reader.ReadBit();
		bool hasDst = reader.ReadBit();
		bool hasOrientation = reader.ReadBit();
		bool hasMapID = reader.ReadBit();
		uint32 nameLength = reader.ReadBits(7);

		reader.ResetBitReader();

		if (hasSrc)
			targetData.SrcLocation = ReadLocation(reader);

		if (hasDst)
			targetData.DstLocation = ReadLocation(reader);

		if (hasOrientation)
			targetData.Orientation = reader.ReadFloat();

		if (hasMapID)
			targetData.MapID = reader.ReadUInt32();

		targetData.Name = reader.ReadWoWString(nameLength);
	}

	TargetLocation Parser::ReadLocation(BitReader& reader)
	{
		TargetLocation loc;
		loc.Transport = ReadPackedGuid128(reader);
		loc.X = reader.ReadFloat();
		loc.Y = reader.ReadFloat();
		loc.Z = reader.ReadFloat();
		return loc;
	}
}