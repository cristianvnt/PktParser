#include "pchdef.h"

#include "Reader/PktFileReader.h"
#include "Database/Database.h"
#include "Parser/ParallelProcessor.h"
#include "VersionFactory.h"
#include "Misc/BuildRegistry.h"

using namespace PktParser;
using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Versions;

using Stats = PktParser::ParallelProcessor::Stats;

int main(int argc, char* argv[])
{
	Logger::Instance().Init("pkt_parser.log");

	if (argc < 2)
	{
		LOG("Usage: {} <path-to-pkt-file>", argv[0]);
		return 1;
	}

#ifdef SYNC_PARSING_MODE
	LOG("Running in SYNC mode");
#else
	LOG("Running in PARALLEL mode");
#endif	
	
	try
	{
		Db::Database db(100000);
		SeedBuildMappings(db);

		BuildRegistry::Initialize(db);

		PktFileReader reader(argv[1]);
		reader.ParseFileHeader();
		uint32 build = reader.GetFileHeader().clientBuild;
		
		if (!VersionFactory::IsSupported(build))
		{
			LOG("ERROR: Build {} is not supported!", build);
            return 1;
		}
		
		VersionContext ctx = VersionFactory::Create(build);
		LOG("Pkt build {} - Using parser for build {}", build, ctx.Build);

#ifdef SYNC_PARSING_MODE
		auto startTime = std::chrono::high_resolution_clock::now();
		size_t parsedCount = 0;
		size_t skippedCount = 0;
		size_t failedCount = 0;

		while (true)
		{
			std::optional<Pkt> pktOpt = reader.ReadNextPacket();
			if (!pktOpt.has_value())
				break;

			Pkt const& pkt = pktOpt.value();
			ParserMethod method = ctx.Parser->GetParserMethod(pkt.header.opcode);
            if (!method)
			{
				skippedCount++;
				continue;
			}
			
			char const* opcodeName = ctx.Parser->GetOpcodeName(pkt.header.opcode);
			try
			{
				BitReader packetReader = pkt.CreateReader();
				json packetData = method(packetReader);
				json fullPacket = ctx.Serializer->SerializeFullPacket(pkt.header, opcodeName, build, pkt.pktNumber, packetData);
				db.StorePacket(fullPacket);
				parsedCount++;

				if (parsedCount % 10000 == 0)
					LOG("Processed {} packets...", parsedCount);
			}
			catch (std::exception const& e)
			{
				LOG("Failed to parse packet {} OP {}: {}", pkt.pktNumber, opcodeName, e.what());
				failedCount++;
			}
		}

		LOG("Parsing complete, flushing...");
		db.Flush();

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

		LOG(">>>>> PARSE COMPLETE <<<<<");
		LOG("Parsed: {}, Skipped: {}, Failed: {}", parsedCount, skippedCount, failedCount);
        LOG("DB Stats: {} inserted, {} failed", db.GetTotalInserted(), db.GetTotalFailed());
		LOG("Total time: {}ms ({:.2f} seconds)", duration.count(), duration.count() / 1000.0);
#else
		Stats stats = ParallelProcessor::ProcessAllPackets(reader, ctx, db);

		LOG(">>>>> PARSE COMPLETE <<<<<");
        LOG("Parsed: {}, Skipped: {}, Failed: {}", stats.ParsedCount, stats.SkippedCount, stats.FailedCount);
        LOG("DB Stats: {} inserted, {} failed", db.GetTotalInserted(), db.GetTotalFailed());
        LOG("Total time: {}ms ({:.2f} seconds)", stats.TotalTime, stats.TotalTime / 1000.0);
#endif

		VersionFactory::Destroy(ctx);
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