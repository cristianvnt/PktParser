#ifndef LOGGER_H
#define LOGGER_H

#include <fmt/core.h>
#include <fstream>
#include <string>

class Logger
{
private:
	Logger() = default;
	std::ofstream _logFile;
	std::mutex _mutex;

	Logger(Logger const&) = delete;
	Logger& operator=(Logger const&) = delete;

public:
	static Logger& Instance()
	{
		static Logger instance;
		return instance;
	}

	void Init(std::string const& filename)
	{
		_logFile.open(filename, std::ios::out | std::ios::trunc);
		if (!_logFile.is_open())
			fmt::print(stderr, "Failed to open log file: {}\n", filename);
	}

	template<typename... Args>
	void Log(fmt::format_string<Args...> fmtStr, Args&&... args)
	{
		std::string message = fmt::format(fmtStr, std::forward<Args>(args)...);

		std::lock_guard<std::mutex> lock(_mutex);

		fmt::print("{}\n", message);

		if (!_logFile.is_open())
			return;

		_logFile << message << "\n";
		_logFile.flush();
	}

	~Logger()
	{
		if (_logFile.is_open())
			_logFile.close();
	}
};

#define LOG(...) Logger::Instance().Log(__VA_ARGS__)

#endif // !LOGGER_H
