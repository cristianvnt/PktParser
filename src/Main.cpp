#include "pchdef.h"

#include "Reader/PktFileReader.h"
#include "Database/Database.h"
#include "Parser/ParallelProcessor.h"
#include "VersionFactory.h"
#include "Database/BuildInfo.h"
#include "Database/OpcodeCache.h"

using namespace PktParser;
using namespace PktParser::Reader;
using namespace PktParser::Misc;
using namespace PktParser::Versions;
using namespace PktParser::Db;

using Stats = PktParser::ParallelProcessor::Stats;

int main(int argc, char* argv[])
{
	Logger::Instance().Init("pkt_parser.log");

	if (argc < 2)
	{
		LOG("Usage: {} <path-to-pkt-file> [--parser-version V11_2_5_63506]", argv[0]);
        return 1;
	}

	std::string pktFilePath = argv[1];
    std::string parserVersion = "";
    
    for (int i = 2; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "--parser-version" && i + 1 < argc)
        {
            parserVersion = argv[i + 1];
            i++;
        }
    }

#ifdef SYNC_PARSING_MODE
	LOG("Running in SYNC mode");
#else
	LOG("Running in PARALLEL mode");
#endif
	
	try
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);

		BuildInfo::Instance().Initialize();

		Database db;

		PktFileReader reader(pktFilePath.c_str());
		reader.ParseFileHeader();
		uint32 build = reader.GetFileHeader().clientBuild;
		
		if (parserVersion.empty())
		{
			auto mapping = BuildInfo::Instance().GetMapping(build);
            if (!mapping.has_value())
            {
                LOG("ERROR: Build {} is not supported!", build);
                return 1;
            }
			parserVersion = mapping->ParserVersion;
			LOG("Pkt build {} (patch {}) - Using parser: {}", build, mapping->PatchVersion, parserVersion);
		}
		else
			LOG("Pkt build {} - Using forced parser: {}", build, parserVersion);

		OpcodeCache::Instance().LoadFromDatabase(parserVersion);
		if (OpcodeCache::Instance().GetOpcodeCount() == 0)
		{
			LOG("SKIP: No opcodes loaded for parser {} - skiping file", parserVersion);
			curl_global_cleanup();
			return 0;
		}

		if (!VersionFactory::IsSupported(build))
        {
            LOG("ERROR: Build {} is not supported!", build);
            return 1;
        }

#ifdef SYNC_PARSING_MODE
		VersionContext ctx = VersionFactory::Create(build);
		LOG("Parser initialized for build {}", ctx.Build);

		auto startTime = std::chrono::high_resolution_clock::now();
		size_t parsedCount = 0;
		size_t skippedCount = 0;
		size_t failedCount = 0;
		
		std::string srcFile = reader.GetFilePath();
		CassUuid fileId = db.GenerateFileId();

		while (true)
		{
			std::optional<Pkt> pktOpt = reader.ReadNextPacket();
			if (!pktOpt.has_value())
				break;

			Pkt const& pkt = *pktOpt;
			char const* opcodeName = ctx.Parser->GetOpcodeName(pkt.header.opcode);
			
			try
			{
				BitReader pktReader = pkt.CreateReader();
				std::optional<json> pktDataOpt = ctx.Parser->ParsePacket(pkt.header.opcode, pktReader);
				if (!pktDataOpt)
				{
					skippedCount++;
					continue;
				}

				db.StorePacket(pkt.header, opcodeName, build, pkt.pktNumber, std::move(*pktDataOpt), srcFile, fileId);
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
		Stats stats = ParallelProcessor::ProcessAllPackets(reader, db);

		LOG(">>>>> PARSE COMPLETE <<<<<");
        LOG("Parsed: {}, Skipped: {}, Failed: {}", stats.ParsedCount, stats.SkippedCount, stats.FailedCount);
        LOG("DB Stats: {} inserted, {} failed", db.GetTotalInserted(), db.GetTotalFailed());
        LOG("Total time: {}ms ({:.2f} seconds)", stats.TotalTime, stats.TotalTime / 1000.0);
#endif
		curl_global_cleanup();
	}
	catch (std::exception const& e)
	{
		LOG("Error: {}", e.what());
		return 1;
	}
}