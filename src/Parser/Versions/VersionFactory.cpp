#include "pchdef.h"
#include "VersionFactory.h"

#include "V11_2_5_64502/Parser.h"
#include "V11_2_7_64632/Parser.h"

#include "V11_2_5_64502/JsonSerializer.h"
#include "V11_2_7_64632/JsonSerializer.h"

using namespace PktParser::Misc;

namespace PktParser::Versions
{
    VersionContext VersionFactory::Create(uint32 build)
    {
        VersionContext ctx{};
        ctx.Build = build;
        BuildMappings const* mapping = BuildRegistry::GetMappings(build);
        if (!mapping)
        {
            ctx.Parser = nullptr;
            ctx.Serializer = nullptr;
            return ctx;
        }

        ctx.Patch = mapping->Patch;

        if (mapping->ParserVersion == "V11_2_5_64502")
        {
            ctx.Parser = new V11_2_5_64502::Parser();
            ctx.Serializer = new V11_2_5_64502::JsonSerializer();
        }
        else if (mapping->ParserVersion == "V11_2_7_64632")
        {
            ctx.Parser = new V11_2_7_64632::Parser();
            ctx.Serializer = new V11_2_7_64632::JsonSerializer();
        }
        else if (mapping->ParserVersion == "V11_2_7_64877")
        {
            // TODO
        }
        else
        {
            LOG("ERROR: Unknown parser version: {}", mapping->ParserVersion);
            ctx.Parser = nullptr;
            ctx.Serializer = nullptr;
        }

        return ctx;
    }

    void VersionFactory::Destroy(VersionContext& ctx)
    {
        delete ctx.Parser;
        delete ctx.Serializer;
        ctx.Parser = nullptr;
        ctx.Serializer = nullptr;
    }

    bool VersionFactory::IsSupported(uint32 build)
    {
        return BuildRegistry::IsSupported(build);
    }
}