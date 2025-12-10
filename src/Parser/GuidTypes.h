#ifndef GUID_TYPES_H
#define GUID_TYPES_H

#include "Misc/Define.h"

namespace PktParser
{
	enum class GuidType : uint8
	{
		Null				= 0,
		Uniq				= 1,
		Player				= 2,
		Item				= 3,
		WorldTransaction	= 4,
		StaticDoor			= 5,
		Transport			= 6,
		Conversation		= 7,
		Creature			= 8,
		Vehicle				= 9,
		Pet					= 10,
		GameObject			= 11,
		DynamicObject		= 12,
		AreaTrigger			= 13,
		Corpse				= 14,
		LootObject			= 15,
		SceneObject			= 16,
		Scenario			= 17,
		AIGroup				= 18,
		DynamicDoor			= 19,
		ClientActor			= 20,
		Vignette			= 21,
		CallForHelp			= 22,
		AIResource			= 23,
		AILock				= 24,
		AILockTicket		= 25,
		ChatChannel			= 26,
		Party				= 27,
		Guild				= 28,
		WowAccount			= 29,
		BNetAccount			= 30,
		GMTask				= 31,
		MobileSession		= 32,
		RaidGroup			= 33,
		Spell				= 34,
		Mail				= 35,
		WebObj				= 36,
		LFGObject			= 37,
		LFGList				= 38,
		UserRouter			= 39,
		PVPQueueGroup		= 40,
		UserClient			= 41,
		PetBattle			= 42,
		UniqUserClient		= 43,
		BattlePet			= 44,
		CommerceObj			= 45,
		ClientSession		= 46,
		Cast				= 47,
		ClientConnection	= 48,
		ClubFinder			= 49,
		ToolsClient			= 50,
		WorldLayer			= 51,
		ArenaTeam			= 52,
		Invalid				= 63
	};

	inline const char* GuidTypeToString(GuidType type)
	{
		switch (type)
		{
		case GuidType::Null:             return "Null";
		case GuidType::Uniq:             return "Uniq";
		case GuidType::Player:           return "Player";
		case GuidType::Item:             return "Item";
		case GuidType::WorldTransaction: return "WorldTransaction";
		case GuidType::StaticDoor:       return "StaticDoor";
		case GuidType::Transport:        return "Transport";
		case GuidType::Conversation:     return "Conversation";
		case GuidType::Creature:         return "Creature";
		case GuidType::Vehicle:          return "Vehicle";
		case GuidType::Pet:              return "Pet";
		case GuidType::GameObject:       return "GameObject";
		case GuidType::DynamicObject:    return "DynamicObject";
		case GuidType::AreaTrigger:      return "AreaTrigger";
		case GuidType::Corpse:           return "Corpse";
		case GuidType::LootObject:       return "LootObject";
		case GuidType::SceneObject:      return "SceneObject";
		case GuidType::Scenario:         return "Scenario";
		case GuidType::AIGroup:          return "AIGroup";
		case GuidType::DynamicDoor:      return "DynamicDoor";
		case GuidType::ClientActor:      return "ClientActor";
		case GuidType::Vignette:         return "Vignette";
		case GuidType::CallForHelp:      return "CallForHelp";
		case GuidType::AIResource:       return "AIResource";
		case GuidType::AILock:           return "AILock";
		case GuidType::AILockTicket:     return "AILockTicket";
		case GuidType::ChatChannel:      return "ChatChannel";
		case GuidType::Party:            return "Party";
		case GuidType::Guild:            return "Guild";
		case GuidType::WowAccount:       return "WowAccount";
		case GuidType::BNetAccount:      return "BNetAccount";
		case GuidType::GMTask:           return "GMTask";
		case GuidType::MobileSession:    return "MobileSession";
		case GuidType::RaidGroup:        return "RaidGroup";
		case GuidType::Spell:            return "Spell";
		case GuidType::Mail:             return "Mail";
		case GuidType::WebObj:           return "WebObj";
		case GuidType::LFGObject:        return "LFGObject";
		case GuidType::LFGList:          return "LFGList";
		case GuidType::UserRouter:       return "UserRouter";
		case GuidType::PVPQueueGroup:    return "PVPQueueGroup";
		case GuidType::UserClient:       return "UserClient";
		case GuidType::PetBattle:        return "PetBattle";
		case GuidType::UniqUserClient:   return "UniqUserClient";
		case GuidType::BattlePet:        return "BattlePet";
		case GuidType::CommerceObj:      return "CommerceObj";
		case GuidType::ClientSession:    return "ClientSession";
		case GuidType::Cast:             return "Cast";
		case GuidType::ClientConnection: return "ClientConnection";
		case GuidType::ClubFinder:       return "ClubFinder";
		case GuidType::ToolsClient:      return "ToolsClient";
		case GuidType::WorldLayer:       return "WorldLayer";
		case GuidType::ArenaTeam:        return "ArenaTeam";
		case GuidType::Invalid:          return "Invalid";
		default:                         return "Unknown";
		}
	}

	inline bool GuidTypeHasEntry(GuidType type)
	{
		switch (type)
		{
		case GuidType::Creature:
		case GuidType::GameObject:
		case GuidType::Pet:
		case GuidType::Vehicle:
		case GuidType::AreaTrigger:
		case GuidType::Cast:
			return true;
		default:
			return false;
		}
	}
}

#endif // !GUID_TYPES_H