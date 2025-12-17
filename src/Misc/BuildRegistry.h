#ifndef BUILD_REGISTRY_H
#define BUILD_REGISTRY_H

#include "Define.h"

#include <string>
#include <unordered_map>
#include <mutex>

namespace PktParser::Db { class Database; }

namespace PktParser::Misc
{
    struct BuildMappings
    {
        std::string Patch;
        uint32 Build;
        int64 DeployTimestamp;
        std::string ParserVersion;
    };

    class BuildRegistry
    {
    private:
        static std::unordered_map<uint32, BuildMappings> _mappings;
        static std::mutex _mutex;
        static bool _initialized;

        static void LoadFromDb(Db::Database& db);

    public:
        static void Initialize(Db::Database& db);
        static BuildMappings const* GetMappings(uint32 build);
        static bool IsSupported(uint32 build);
        static void Reload(Db::Database& db);
    };
}

#endif