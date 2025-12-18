#ifndef BUILD_INFO_H
#define BUILD_INFO_H

#include "Misc/Define.h"
#include <string>
#include <optional>
#include <unordered_map>

namespace PktParser::Db
{
    struct BuildMapping
    {
        uint32 BuildNumber;
        std::string PatchVersion;
        std::string ParserVersion;
    };

    class BuildInfo
    {
    private:
        BuildInfo() = default;
        
        std::unordered_map<uint32, BuildMapping> _mappings;

    public:
        static BuildInfo& Instance();

        void Initialize();
        std::optional<BuildMapping> GetMapping(uint32 BuildNumber) const;
        bool IsSupported(uint32 BuildNumber) const;
    };
}

#endif