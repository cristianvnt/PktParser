#include <fstream>
#include <cstdint>
#include <filesystem>
#include <fmt/core.h>

#include "Define.h"
#include "Logger.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	Logger::Instance().Init("pkt_parser.log");

	if (argc < 2)
	{
		fmt::print(stderr, "Usage: {} <pkt_file>\n", argv[0]);
		fmt::print(stderr, "Or drag and drop a .pkt file onto the executable\n");
		return 1;
	}

	fs::path filepath = argv[1];
	LOG("Opening file: {}", filepath.string());

	std::ifstream file(filepath, std::ios::binary);

	if (!file.is_open())
	{
		LOG("Failed to open file: {}", filepath.string());
		return 1;
	}

	char magic[3];
	file.read(magic, 3);

	if (!std::equal(magic, magic + 3, "PKT"))
	{
		LOG("Not a valid PKT file!!!");
		return 1;
	}

	LOG("Found PKT magik: PKT");

	uint16 version;
	file.read(reinterpret_cast<char*>(&version), sizeof(version));
	LOG("Version: 0x{:04X}", version);

	uint8 snifferId;
	file.read(reinterpret_cast<char*>(&snifferId), sizeof(snifferId));
	LOG("Sniffer ID: 0x{:02X} ({})", snifferId, static_cast<char>(snifferId));

	uint32 clientBuild;
	file.read(reinterpret_cast<char*>(&clientBuild), sizeof(clientBuild));
	LOG("Client Build: {}", clientBuild);

	char locale[5] = {0};
	file.read(locale, 4);
	LOG("Locale: {}", std::string_view(locale, 4));

	file.seekg(40, std::ios::cur);
	LOG("skippin 40 bytes of session key");

	uint32 startTimeUnix;
	file.read(reinterpret_cast<char*>(&startTimeUnix), sizeof(startTimeUnix));
	LOG("start time (unix): {}", startTimeUnix);

	uint32 startTickCount;
	file.read(reinterpret_cast<char*>(&startTickCount), sizeof(startTickCount));
	LOG("start tick count: {}", startTickCount);

	int32 additionalLength;
	file.read(reinterpret_cast<char*>(&additionalLength), sizeof(additionalLength));
	LOG("additional data length: {}", additionalLength);

	if (additionalLength > 0)
	{
		file.seekg(additionalLength, std::ios::cur);
		LOG("Skipped {} bytes of additional data", additionalLength);
	}

	LOG("Parse COMPLETE YAY");

	file.close();
	
#ifdef _WIN32
	std::cin.get();
#endif
}