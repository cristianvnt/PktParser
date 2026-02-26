#ifndef DATABASE_CONFIG_H
#define DATABASE_CONFIG_H

#include <mutex>

namespace PktParser::Db
{
    class Config
    {
    private:
        static void LoadEnvImpl();
        static std::once_flag _initFlag;
        
    public:
        static void LoadEnv() { std::call_once(_initFlag, LoadEnvImpl); }
        static std::string GetPostgresConnectionString();
        static std::string GetCassandraHost();
        static std::string GetCassandraKeyspace();
        static std::string GetEnv(char const* name);
    };
}

#endif