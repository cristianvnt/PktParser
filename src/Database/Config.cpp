#include "pchdef.h"
#include "Config.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

namespace PktParser::Db
{
    std::once_flag Config::_initFlag;

    void Config::LoadEnvImpl()
    {
        std::ifstream envFile(".env");
        if (!envFile.is_open())
        {
            LOG("WARNING: .env file not found, using defaults");
            return;
        }

        std::string line;
        while (std::getline(envFile, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            size_t pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            setenv(key.c_str(), value.c_str(), 0);
        }
    }

    std::string Config::GetPostgresConnectionString()
    {
        LoadEnv();

        char const* host = std::getenv("POSTGRES_HOST");
        char const* port = std::getenv("POSTGRES_PORT");
        char const* db = std::getenv("POSTGRES_DB");
        char const* user = std::getenv("POSTGRES_USER");
        char const* pass = std::getenv("POSTGRES_PASSWORD");

        std::ostringstream oss;
        oss << "host=" << (host ? host : "localhost")
            << " port=" << (port ? port : "5432")
            << " dbname=" << (db ? db : "wow_metadata")
            << " user=" << (user ? user : "wowparser")
            << " password=" << (pass ? pass : "");

        return oss.str();
    }

    std::string Config::GetCassandraHost()
    {
        LoadEnv();
        char const* host = std::getenv("CASSANDRA_HOST");
        return host ? host : "127.0.0.1";
    }

    std::string Config::GetCassandraKeyspace()
    {
        LoadEnv();
        char const* keyspace = std::getenv("CASSANDRA_KEYSPACE");
        return keyspace ? keyspace : "wow_packets";
    }
}