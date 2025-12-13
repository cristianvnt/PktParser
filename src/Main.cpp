#include <fmt/core.h>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <future>

#include "Misc/Utilities.h"
#include "Misc/Logger.h"
#include "Reader/BitReader.h"
#include "Reader/PktFileReader.h"
#include "Parser/Parser.h"
#include "Parser/PktHandler.h"
#include "Parser/JsonSerializer.h"
#include "Database/Database.h"

using namespace PktParser;
using namespace PktParser::Reader;
using namespace PktParser::Misc;

int main(int argc, char* argv[])
{
	Logger::Instance().Init("pkt_parser.log");

	if (argc < 2)
	{
		LOG("Usage: {} <path-to-pkt-file>", argv[0]);
		return 1;
	}
	
	try
	{
		Db::Database db;

		PktFileReader reader(argv[1]);
		reader.ParseFileHeader();
		
		PktRouter router;
		Parser::RegisterHandlers(router);

		auto startTime = std::chrono::high_resolution_clock::now();

		uint32 build = reader.GetFileHeader().clientBuild;
		std::optional<Pkt> pktOpt = reader.ReadNextPacket();
		size_t parsedCount = 0;
		size_t skippedCount = 0;

		while (pktOpt.has_value())
		{
			Pkt const& pkt = pktOpt.value();

			if (!IsKnownOpcode(pkt.header.opcode))
			{
				skippedCount++;
				pktOpt = reader.ReadNextPacket();
				continue;
			}
			uint32 pktNumber = reader.GetPacketNumber() - 1;

			BitReader packetReader = pkt.CreateReader();
			json packetData = router.HandlePacket(pkt.header.opcode, packetReader, pktNumber);
			json fullPacket = JsonSerializer::SerializeFullPacket(pkt.header, build, pktNumber, packetData);

			db.StorePacket(fullPacket);
			parsedCount++;

			if (parsedCount % 1000 == 0)
				LOG("Processed {} packets...", parsedCount);

			pktOpt = reader.ReadNextPacket();
		}

		LOG("Parsing complete, flushing...");
		db.Flush();

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		LOG(">>>>> PARSE COMPLETE - {} packets parsed, {} skipped <<<<<", parsedCount, skippedCount);
		LOG("Parsed: {}, Skipped: {}", parsedCount, skippedCount);
        LOG("DB Stats: {} inserted, {} failed", db.GetTotalInserted(), db.GetTotalFailed());
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