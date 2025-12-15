#ifndef VERSION_FACTORY_H
#define VERSION_FACTORY_H

#include "IVersionParser.h"
#include "IJsonSerializer.h"

namespace PktParser::Versions
{
    struct VersionContext
    {
        IVersionParser* Parser;
        IJsonSerializer* Serializer;
        uint32 Build;
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