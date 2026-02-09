#include "pchdef.h"
#include "VersionFactory.h"
#include "Database/BuildInfo.h"

#include "V11_2_5_63506/Parser.h"
#include "V11_2_7_64632/Parser.h"
#include "V11_2_7_64877/Parser.h"
#include "V12_0_0_65390/Parser.h"

#include "V11_2_5_63506/JsonSerializer.h"
#include "V11_2_7_64632/JsonSerializer.h"
#include "V11_2_7_64877/JsonSerializer.h"
#include "V12_0_0_65390/JsonSerializer.h"

using namespace PktParser::Misc;

namespace PktParser::Versions
{
    VersionContext VersionFactory::Create(uint32 build)
    {
        VersionContext ctx{};
        ctx.Build = build;
        auto mapping = Db::BuildInfo::Instance().GetMapping(build);
        if (!mapping.has_value())
        {
            ctx.Parser = nullptr;
            ctx.Serializer = nullptr;
            return ctx;
        }

        ctx.Patch = mapping->PatchVersion;

        if (mapping->ParserVersion == "V11_2_5_63506")
        {
            ctx.Parser = new V11_2_5_63506::Parser();
            ctx.Serializer = new V11_2_5_63506::JsonSerializer();
        }
        else if (mapping->ParserVersion == "V11_2_7_64632")
        {
            ctx.Parser = new V11_2_7_64632::Parser();
            ctx.Serializer = new V11_2_7_64632::JsonSerializer();
        }
        else if (mapping->ParserVersion == "V11_2_7_64877")
        {
            ctx.Parser = new V11_2_7_64877::Parser();
            ctx.Serializer = new V11_2_7_64877::JsonSerializer();
        }
        else if (mapping->ParserVersion == "V12_0_0_65390")
        {
            ctx.Parser = new V12_0_0_65390::Parser();
            ctx.Serializer = new V12_0_0_65390::JsonSerializer();
        }
        else
        {
            LOG("ERROR: Unknown parser version: {}", mapping->ParserVersion);
            ctx.Parser = nullptr;
            ctx.Serializer = nullptr;
        }

        return ctx;
    }

    bool VersionFactory::IsSupported(uint32 build)
    {
        return Db::BuildInfo::Instance().IsSupported(build);
    }
}