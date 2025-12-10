#include "Parser.h"
#include "Misc/Logger.h"
#include "Misc/WowGuid.h"
#include "Misc/Utilities.h"
#include "PktStructures.h"
#include "PktHandler.h"
#include "JsonSerializer.h"
#include "Enums/TargetFlags.h"
#include "Enums/CastFlags.h"
#include "Enums/SpellMissType.h"

#include <vector>
#include <fmt/core.h>
#include <chrono>

namespace PktParser
{
	void Parser::RegisterHandlers(PktRouter& router)
	{
		router.RegisterHandler(Opcode::SMSG_AUTH_CHALLENGE, ParseAuthChallenge);
		router.RegisterHandler(Opcode::SMSG_SPELL_GO, ParseSpellGo);
		router.RegisterHandler(Opcode::SMSG_UPDATE_WORLD_STATE, ParseUpdateWorldState);
	}

	json Parser::ParseAuthChallenge(BitReader& reader)
	{
		reader.ReadUInt32(); // skip opcode
		reader.ResetBitReader();
		AuthChallengeData const* authData = reader.ReadChunk<AuthChallengeData>();

		return JsonSerializer::SerializeAuthChallenge(authData);
	}

	json Parser::ParseSpellGo(BitReader& reader)
	{
		reader.ReadUInt32(); // skip opcode

		WowGuid128 casterGUID = ReadPackedGuid128(reader);
		LOG("CasterGUID: {:016X}{:016X}", casterGUID.High, casterGUID.Low);

		WowGuid128 casterUnit = ReadPackedGuid128(reader);
		LOG("CasterUnit: {:016X}{:016X}", casterUnit.High, casterUnit.Low);

		WowGuid128 castID = ReadPackedGuid128(reader);
		LOG("CastID: {:016X}{:016X}", castID.High, castID.Low);

		WowGuid128 originalCastID = ReadPackedGuid128(reader);
		LOG("OriginalCastID: {:016X}{:016X}", originalCastID.High, originalCastID.Low);

		LOG("Byte position after 4 GUIDs: {}", reader.GetBytePosition());

		SpellHealPrediction defaultPrediction = { 0, 0 };
		SpellHealPrediction const* healPrediction = &defaultPrediction;

		SpellCastFixedData const* basicInfo = reader.ReadChunk<SpellCastFixedData>();
		LOG("Byte position after basicInfo: {}", reader.GetBytePosition());

		return json::object();
	}

	json Parser::ParseUpdateWorldState(BitReader& reader)
	{
		reader.ReadUInt32(); // skip opcode

		WorldStateInfo const* worldStateInfo = reader.ReadChunk<WorldStateInfo>();

		reader.ResetBitReader();
		bool hidden = reader.ReadBit();

		return JsonSerializer::SerializeUpdateWorldState(worldStateInfo, hidden);
	}

}