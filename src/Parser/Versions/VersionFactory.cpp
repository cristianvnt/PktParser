#include "VersionFactory.h"

#include "Misc/Logger.h"
#include "V11_2_5_64502/Parser.h"
#include "V11_2_7_64632/Parser.h"

#include "V11_2_5_64502/JsonSerializer.h"
#include "V11_2_7_64632/JsonSerializer.h"

namespace PktParser::Versions
{
    VersionContext VersionFactory::Create(uint32 build)
    {
        VersionContext ctx{};
        ctx.Build = build;

        switch (build)
        {
            case 63834:
            case 64270:
            case 64502:
                ctx.Parser = new V11_2_5_64502::Parser();
                ctx.Serializer = new V11_2_5_64502::JsonSerializer();
                break;
            case 64632:
            case 64704:
                ctx.Parser = new V11_2_7_64632::Parser();
                ctx.Serializer = new V11_2_7_64632::JsonSerializer();
                break;
            default:
                ctx.Parser = nullptr;
                ctx.Serializer = nullptr;
                break;
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
        switch (build)
        {
            case 63834:
            case 64270:
            case 64502:
            case 64632:
            case 64704:
                return true;
            default:
                return false;
        }
    }
}