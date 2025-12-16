#ifndef VERSION_FACTORY_H
#define VERSION_FACTORY_H

#include "IVersionParser.h"
#include "Common/BaseJsonSerializer.h"

namespace PktParser::Versions
{
    struct VersionContext
    {
        IVersionParser* Parser;
        Common::BaseJsonSerializer* Serializer;
        uint32 Build;
        std::string Patch;
    };

    class VersionFactory
    {
    public:
        static VersionContext Create(uint32 build);
        static void Destroy(VersionContext& ctx);
        static bool IsSupported(uint32 build);
    };
}

#endif