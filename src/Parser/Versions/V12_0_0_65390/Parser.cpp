#include "Parser.h"
#include "pchdef.h"

#include "Opcodes.h"
#include "RegisterHandlers.inl"
#include "Handlers/SpellHandler.h"
#include "Serializers/SpellSerializer.h"
#include "SearchFields/SpellSearchFields.h"
#include "JsonWriter.h"

using namespace PktParser::Common;

namespace PktParser::V12_0_0_65390
{
	using namespace Handlers;
	using namespace Structures;
	using namespace Serializers;
	using namespace SearchFields;

	Parser::Parser() : _registry { this }
	{
		_registry.Reserve(REGISTRY_RESERVE_SIZE);
		RegisterAllHandlers(this, _registry);
	}

    std::optional<ParseResult> Parser::ParsePacket(uint32 opcode, BitReader &reader)
    {
		reader.Skip(4);
		return _registry.Dispatch(opcode, reader);
    }

    ParseResult Parser::HandleAuthChallenge([[maybe_unused]] BitReader &reader)
    {
		return ParseResult{ "", nullptr };
    }

    ParseResult Parser::HandleSpellStart(BitReader &reader)
    {
		SpellCastData data = ParseSpellCastData(reader);

		JsonWriter w(SPELL_CAST_JSON_RESERVE);
		SerializeSpellData(w, data);

		SpellSearchFields fields = FillSpellFields(data);

		return ParseResult{ w.TakeString(), new SpellSearchFields(fields) };
    }

    ParseResult Parser::HandleSpellGo(BitReader &reader)
    {
        SpellCastData data = ParseSpellCastData(reader);

		JsonWriter w(SPELL_CAST_JSON_RESERVE);
		SerializeSpellData(w, data);

		SpellSearchFields fields = FillSpellFields(data);

		return ParseResult{ w.TakeString(), new SpellSearchFields(fields) };
    }
	
    ParseResult Parser::HandleUpdateWorldState([[maybe_unused]] BitReader &reader)
    {
        return ParseResult{ "", nullptr };
    }
}