#include "pchdef.h"
#include "Parser.h"

#include "Opcodes.h"
#include "RegisterHandlers.inl"
#include "Common/Parsers/SpellHandlers.inl"

using namespace PktParser::Common::Parsers;
using namespace PktParser::Versions;
using namespace PktParser::Db;

namespace PktParser::Versions::V11_2_7_64632
{
    Parser::Parser()
    {
        _registry.Reserve(REGISTRY_RESERVE_SIZE);
        RegisterAllHandlers(this, _registry);
    }

    json Parser::ParseSpellStart(BitReader& reader)
	{
		return SpellHandlers::ParseSpellCastData(reader, &_serializer, SpellHandlers::ParseSpellTargetData<SpellHandlers::SpellTargetVersion::Housing>);
	}

	json Parser::ParseSpellGo(BitReader& reader)
	{
		return SpellHandlers::ParseSpellCastData(reader, &_serializer, SpellHandlers::ParseSpellTargetData<SpellHandlers::SpellTargetVersion::Housing>);
	}
}