#ifndef VERSION_FACTORY_H
#define VERSION_FACTORY_H

#include "IVersionParser.h"
#include "Common/BaseJsonSerializer.h"

namespace PktParser::Versions
{
    struct VersionContext
    {
        IVersionParser* Parser = nullptr;
        uint32 Build = 0;
        std::string Patch;

        ~VersionContext()
        {
            delete Parser;
        }

        VersionContext() = default;
        VersionContext(VersionContext&& other) noexcept
            : Parser(other.Parser), Build(other.Build), Patch(std::move(other.Patch))
        {
            other.Parser = nullptr;
        }

        VersionContext(VersionContext const&) = delete;
        VersionContext& operator=(VersionContext const&) = delete;
    };

    class VersionFactory
    {
    public:
        static VersionContext Create(uint32 build);
        static bool IsSupported(uint32 build);
    };
}

#endif