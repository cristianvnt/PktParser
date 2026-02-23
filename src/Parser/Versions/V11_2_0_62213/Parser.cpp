#include "pchdef.h"
#include "Parser.h"

#include "Opcodes.h"
#include "RegisterHandlers.inl"
#include "Common/Parsers/SpellHandlers.inl"

using namespace PktParser::Common::Parsers;
using namespace PktParser::Common;
using namespace PktParser::Versions;
using namespace PktParser::Enums;
using namespace PktParser::Db;

namespace PktParser::Versions::V11_2_0_62213
{
	Parser::Parser()
	{
		_registry.Reserve(REGISTRY_RESERVE_SIZE);
		RegisterAllHandlers(this, _registry);
	}

    ParseResult Parser::ParseSpellStart(BitReader &reader)
    {
        return SpellHandlers::ParseSpellCastData<SpellTargetVersion::Lower>(reader, &_serializer, SpellHandlers::ParseSpellTargetData<Lower>);
    }

    ParseResult Parser::ParseSpellGo(BitReader& reader)
	{
		return SpellHandlers::ParseSpellCastData<SpellTargetVersion::Lower>(reader, &_serializer, SpellHandlers::ParseSpellTargetData<Lower>);
	}
}