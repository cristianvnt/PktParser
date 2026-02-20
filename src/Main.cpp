#include "pchdef.h"

#include "Reader/PktFileReader.h"
#include "Database/Database.h"
#include "Parser/ParallelProcessor.h"
#include "VersionFactory.h"
#include "Database/BuildInfo.h"
#include "Database/OpcodeCache.h"

#ifdef HAS_DROGON
#include <drogon/HttpAppFramework.h>
#endif

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
		LOG("Server usage: {} --serve", argv[0]);
		LOG("Parser usage: {} <path-to-pkt-file> [--parser-version V11_2_5_63506]", argv[0]);
        return 1;
	}

	std::string pktFilePath = argv[1];
    std::string parserVersion = "";
    bool toCSV = false;
	bool serveRequested = false;

	std::string arg = argv[1];
	if (arg == "--serve")
		serveRequested = true;
	else
    {
		for (int i = 2; i < argc; i++)
		{
			arg = argv[i];
			if (arg == "--parser-version" && i + 1 < argc)
			{
				parserVersion = argv[i + 1];
				i++;
			}
			else if (arg == "--export")
				toCSV = true;
		}
	}

	LOG("Running in PARALLEL mode");
	
	try
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);

		BuildInfo::Instance().Initialize();

		std::optional<Database> db;
		if (!toCSV)
			db.emplace();

		if (serveRequested)
		{
		#ifdef HAS_DROGON
			LOG("Drogon server starting on port 8080...");
			drogon::app().loadConfigFile("drogon_config.json");

			drogon::app().registerHandler("/api/health",
				[](drogon::HttpRequestPtr const&, std::function<void(drogon::HttpResponsePtr const&)>&& callback)
				{
					Json::Value json;
					json["status"] = "OK";
					json["service"] = "PktParser API";
					drogon::HttpResponsePtr res = drogon::HttpResponse::newHttpJsonResponse(json);
					callback(res);
				}, { drogon::Get }
			);

			drogon::app().run();

			curl_global_cleanup();
			return 0;
		#else
			LOG("Drogon not enabled!");
        	return 1;
		#endif
		}

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

		Stats stats = ParallelProcessor::ProcessAllPackets(reader, db ? &(*db) : nullptr, 0, toCSV);

		LOG(">>>>> PARSE COMPLETE <<<<<");
        LOG("Parsed: {}, Skipped: {}, Failed: {}", stats.ParsedCount, stats.SkippedCount, stats.FailedCount);
		if (db)
        	LOG("DB Stats: {} inserted, {} failed", db->GetTotalInserted(), db->GetTotalFailed());
        LOG("Total time: {}ms ({:.2f} seconds)", stats.TotalTime, stats.TotalTime / 1000.0);

		curl_global_cleanup();
	}
	catch (std::exception const& e)
	{
		LOG("Error: {}", e.what());
		return 1;
	}
}