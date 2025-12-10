#ifndef SPELL_CAST_VISUAL_H
#define SPELL_CAST_VISUAL_H

#include "Misc/Define.h"

namespace PktParser::Structures::Packed
{
#pragma pack(push, 1)
	struct SpellCastVisual
	{
		int32 SpellXSpellVisualID;
		int32 ScriptVisualID;
	};
#pragma pack(pop)
}

#endif