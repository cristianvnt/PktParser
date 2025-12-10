#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include "Reader/PktFileReader.h"
#include "Misc/WowGuid.h"
#include "PktStructures.h"
#include "Misc/Utilities.h"
#include "Misc/Logger.h"

#include <nlohmann/json.hpp>
#include <string>
#include <fmt/core.h>

using json = nlohmann::ordered_json;

namespace PktParser
{
	class JsonSerializer
	{
	public:

		static json SerializePacketHead(PktHeader const& header, uint32 build)
		{
			json j;
			j["Direction"] = Utilities::DirectionToString(header.direction);
			j["PacketName"] = GetOpcodeName(header.opcode);
			j["ConnectionIndex"] = header.connectionIndex;
			j["TickCount"] = header.tickCount;
			j["Timestamp"] = Utilities::FormatUnixMilliseconds(header.timestamp);
			j["Opcode"] = fmt::format("0x{:06X}", header.opcode);
			j["Length"] = header.packetLength;
			j["Build"] = build;

			return j;
		}

		static json SerializeGUID(WowGuid128 const& guid)
		{
			json j;
			j["Low"] = guid.Low;
			j["High"] = guid.High;
			j["Full"] = fmt::format("{:016X}{:016X}", guid.High, guid.Low);

			return j;
		}

		static json SerializeAuthChallenge(AuthChallengeData const* data)
		{
			json j;
			j["DosChallenge"] = json::array();
			for (int i = 0; i < 8; i++)
				j["DosChallenge"].push_back(data->DosChallenge[i]);

			std::string challengeHex;
			challengeHex.reserve(64);
			for (int i = 0; i < 32; i++)
				fmt::format_to(std::back_inserter(challengeHex), "{:02X}", data->Challenge[i]);
			j["Challenge"] = challengeHex;
			j["DosZeroBits"] = data->DosZeroBits;

			return j;
		}

		static json SerializeSpellGo(WowGuid128 const& casterGUID, WowGuid128 const& casterUnit, WowGuid128 const& castID, WowGuid128 const& originalCastID,
			SpellCastFixedData const* fixedData, SpellHealPrediction const* healPrediction, WowGuid128 const& beaconGUID,
			std::vector<WowGuid128> const& hitTargets, std::vector<WowGuid128> const& missTargets,
			std::vector<uint8> const& hitStatus, std::vector<SpellMissStatus> const& missStatus, std::vector<uint32> const& remainingPower,
			bool hasRemainingRunes, RuneData const* runeData, std::vector<uint8> const& cooldowns, std::vector<TargetLocation> const& targetPoints,
			uint32 targetFlags, WowGuid128 const& targetUnit, WowGuid128 const& targetItem,
			bool hasSrcLocation, TargetLocation const& srcLocation, bool hasDstLocation, TargetLocation const& dstLocation,
			bool hasOrientation, float orientation, bool hasMapID, uint32 mapID)
		{
			json j;

			j["CasterType"] = GuidTypeToString(casterGUID.GetType());
			j["CasterRealmId"] = casterGUID.GetRealmId();
			j["CasterLow"] = casterGUID.GetLow();
			j["MapId"] = casterGUID.GetMapId();

			if (!castID.IsEmpty())
			{
				j["CastEntry"] = castID.GetEntry();
				j["CastLow"] = castID.GetLow();
			}

			j["SpellID"] = fixedData->SpellID;
			j["VisualID"] = fixedData->Visual.SpellXSpellVisualID;
			j["ScriptVisualID"] = fixedData->Visual.ScriptVisualID;
			j["CastFlags"] = fixedData->CastFlags;
			j["CastFlagsEx"] = fixedData->CastFlagsEx;
			j["CastFlagsEx2"] = fixedData->CastFlagsEx2;
			j["CastTime"] = fixedData->CastTime;

			j["TravelTime"] = fixedData->MissileTrajectory.TravelTime;
			j["Pitch"] = fixedData->MissileTrajectory.Pitch;

			j["AmmoDisplayID"] = fixedData->AmmoDisplayID;
			j["ImmunitySchool"] = fixedData->Immunities.School;
			j["ImmunityValue"] = fixedData->Immunities.Value;

			j["HealPoints"] = healPrediction->Points;
			j["HealType"] = healPrediction->Type;

			j["HitTargetsCount"] = hitTargets.size();
			json hitArray = json::array();
			for (size_t i = 0; i < hitTargets.size(); ++i)
			{
				json t;
				t["Type"] = GuidTypeToString(hitTargets[i].GetType());
				t["Low"] = hitTargets[i].GetLow();
				if (hitTargets[i].HasEntry())
					t["Entry"] = hitTargets[i].GetEntry();
				t["HitResult"] = (i < hitStatus.size()) ? hitStatus[i] : 0;
				hitArray.push_back(t);
			}
			j["HitTargets"] = hitArray;

			j["MissTargetsCount"] = missTargets.size();
			json missArray = json::array();
			for (size_t i = 0; i < missTargets.size(); ++i)
			{
				json t;
				t["Type"] = GuidTypeToString(missTargets[i].GetType());
				t["Low"] = missTargets[i].GetLow();
				if (i < missStatus.size())
				{
					t["MissReason"] = missStatus[i].MissReason;
					t["Reflect"] = missStatus[i].ReflectStatus;
				}
				missArray.push_back(t);
			}
			j["MissTargets"] = missArray;

			if (!remainingPower.empty())
				j["RemainingPower"] = remainingPower;

			if (hasRemainingRunes && runeData)
			{
				j["RuneStart"] = runeData->Start;
				j["RuneCount"] = runeData->Count;
				j["RuneCooldowns"] = cooldowns;
			}

			if (!targetPoints.empty())
			{
				json locArray = json::array();
				for (auto const& loc : targetPoints)
				{
					json l;
					l["X"] = loc.X;
					l["Y"] = loc.Y;
					l["Z"] = loc.Z;
					locArray.push_back(l);
				}
				j["TargetPoints"] = locArray;
			}

			j["TargetFlags"] = targetFlags;

			if (!targetUnit.IsEmpty() && targetUnit.GetType() != GuidType::Null)
			{
				j["TargetUnit"] = targetUnit.GetLow();
				j["TargetUnitType"] = GuidTypeToString(targetUnit.GetType());
			}

			if (!targetItem.IsEmpty() && targetItem.GetType() != GuidType::Null)
				j["TargetItem"] = targetItem.GetLow();

			if (hasDstLocation)
			{
				json dst;
				dst["X"] = dstLocation.X;
				dst["Y"] = dstLocation.Y;
				dst["Z"] = dstLocation.Z;
				j["DstLocation"] = dst;
			}

			if (hasSrcLocation)
			{
				json src;
				src["X"] = srcLocation.X;
				src["Y"] = srcLocation.Y;
				src["Z"] = srcLocation.Z;
				j["SrcLocation"] = src;
			}

			if (hasOrientation)
				j["Orientation"] = orientation;

			if (hasMapID)
				j["TargetMapID"] = mapID;

			//LOG("{}", j.dump(2));
			return j;
		}

		static json SerializeUpdateWorldState(WorldStateInfo const* info, bool hidden)
		{
			json j;
			j["WorldStateId"] = info->VariableID;
			j["Value"] = info->Value;
			j["Hidden"] = hidden;

			return j;
		}

		static json SerializeFullPacket(PktHeader const& header, uint32 build, json const& packetData)
		{
			json j;
			j["Header"] = SerializePacketHead(header, build);
			j["Data"] = packetData;

			return j;
		}
	};
}

#endif // !JSON_SERIALIZER_H
