#include <fmt/core.h>
#include <fstream>
#include <cstdint>
#include <filesystem>

#include "Misc/Utilities.h"
#include "Misc/Logger.h"
#include "Reader/BitReader.h"
#include "Reader/PktFileReader.h"
#include "Parser/Parser.h"
#include "Parser/PktHandler.h"

using namespace PktParser;

int main(int argc, char* argv[])
{
	Logger::Instance().Init("pkt_parser.log");

	if (argc < 2)
	{
		fmt::print(stderr, "Usage: {} <pkt_file>\n", argv[0]);
		fmt::print(stderr, "Or drag and drop a .pkt file onto the executable\n");
		return 1;
	}

	try
	{
		PktFileReader reader(argv[1]);
		reader.ParseFileHeader();
		
		PktRouter router;
		Parser::RegisterHandlers(router);

		LOG("===== Reading Packets =====");
		LOG("Build: {}", reader.GetBuildVersion());

		auto startTime = std::chrono::high_resolution_clock::now();

		std::optional<Pkt> pktOpt = reader.ReadNextPacket();
		size_t parsedCount = 0;

		while (pktOpt.has_value())
		{
			Pkt const& pkt = pktOpt.value();

			/*LOG("{}: {} Opcode 0x{:06X} | Length: {} | ConnIdx: {} | Time: {} | Number: {}",
				Utilities::DirectionToString(pkt.header.direction),
				GetOpcodeName(pkt.header.opcode),
				pkt.header.opcode,
				pkt.header.packetLength - 4,
				pkt.header.connectionIndex,
				Utilities::FormatUnixMilliseconds(pkt.header.timestamp),
				reader.GetPacketNumber() - 1
			);*/

			BitReader packetReader = pkt.CreateReader();
			router.HandlePacket(pkt.header.opcode, packetReader);

			parsedCount++;
			pktOpt = reader.ReadNextPacket();
		}

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		LOG(">>>>> PARSE COMPLETE - {} packets parsed <<<<<", parsedCount);
		LOG("Total time: {}ms ({:.2f} seconds)", duration.count(), duration.count() / 1000.0);
	}
	catch (std::exception const& e)
	{
		LOG("Error: {}", e.what());
		return 1;
	}

#ifdef _WIN32
	std::cin.get();
#endif
}