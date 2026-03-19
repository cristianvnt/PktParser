#pragma once

#include "IVersionParser.h"

namespace PktParser::Versions
{
    struct VersionContext
    {
        IVersionParser* Parser = nullptr;
        uint32 Build = 0;
        std::string Patch;

        VersionContext() = default;
        ~VersionContext() { delete Parser; }

        VersionContext(VersionContext &&other) noexcept : Parser{ other.Parser }, Build{ other.Build }, Patch { std::move(other.Patch) }
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
