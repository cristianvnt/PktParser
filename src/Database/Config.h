#ifndef DATABASE_CONFIG_H
#define DATABASE_CONFIG_H

#include <string>

namespace PktParser::Db
{
    class Config
    {
    private:
        static void LoadEnv();
        static bool _loaded;
        
    public:
        static std::string GetPostgresConnectionString();
        static std::string GetCassandraHost();
        static std::string GetCassandraKeyspace();
    };
}

#endif