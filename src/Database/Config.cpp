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

#ifdef _WIN32
			size_t reqSize = 0;
			getenv_s(&reqSize, nullptr, 0, key.c_str());
			if (reqSize == 0)
				_putenv_s(key.c_str(), value.c_str());
#else
			setenv(key.c_str(), value.c_str(), 0);
#endif
		}
	}

	std::string Config::GetPostgresConnectionString()
	{
		LoadEnv();

		std::string host = GetEnv("POSTGRES_HOST");
		std::string port = GetEnv("POSTGRES_PORT");
		std::string db = GetEnv("POSTGRES_DB");
		std::string user = GetEnv("POSTGRES_USER");
		std::string pass = GetEnv("POSTGRES_PASSWORD");

		std::ostringstream oss;
		oss << "host=" << (host.empty() ? "localhost" : host)
			<< " port=" << (port.empty() ? "5432" : port)
			<< " dbname=" << (db.empty() ? "wow_metadata" : db)
			<< " user=" << (user.empty() ? "wowparser" : user)
			<< " password=" << (pass.empty() ? "" : pass);

		return oss.str();
	}

	std::string Config::GetCassandraHost()
	{
		LoadEnv();
		std::string host = GetEnv("CASSANDRA_HOST");
		return host.empty() ? "127.0.0.1" : host;
	}

	std::string Config::GetCassandraKeyspace()
	{
		LoadEnv();
		std::string keyspace = GetEnv("CASSANDRA_KEYSPACE");
		return keyspace.empty() ? "wow_packets" : keyspace;
	}

	std::string Config::GetEnv(char const* name)
	{
#ifdef _WIN32
		size_t reqSize = 0;
		getenv_s(&reqSize, nullptr, 0, name);
		if (reqSize == 0)
			return {};
		std::string value(reqSize - 1, '\0');
		getenv_s(&reqSize, value.data(), reqSize, name);
		return value;
#else
		char const* val = std::getenv(name);
		return val ? val : std::string{};
#endif
	}

}