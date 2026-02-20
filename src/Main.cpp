#include "pchdef.h"

#include "Reader/PktFileReader.h"
#include "Database/Database.h"
#include "Parser/ParallelProcessor.h"
#include "VersionFactory.h"
#include "IVersionParser.h"
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

	std::string inputPath = argv[1];
    std::string forcedParserVersion = "";
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
				forcedParserVersion = argv[i + 1];
				i++;
			}
			else if (arg == "--export")
				toCSV = true;
		}
	}
	
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

	std::vector<std::filesystem::path> files = Misc::CollectPktFiles(inputPath);
	if (files.empty())
	{
        LOG("No .pkt files found in '{}'", inputPath);
        curl_global_cleanup();
        return 0;
	}
	LOG("Found {} .pkt file(s) to process", files.size());

	std::unordered_map<std::string, VersionContext> versionCache;
	Stats totalStats{};
    auto globalStart = std::chrono::high_resolution_clock::now();

	for (auto const& filePath : files)
	{
		LOG("--- Processing: {} ---", filePath.string());

		try
        {
			PktFileReader reader(filePath.string().c_str());
			reader.ParseFileHeader();
			uint32 build = reader.GetFileHeader().clientBuild;
			
			std::string parserVersion = forcedParserVersion;
			if (parserVersion.empty())
			{
				auto mapping = BuildInfo::Instance().GetMapping(build);
				if (!mapping.has_value())
				{
					LOG("SKIP: Build {} not supported, skipping {}", build, filePath.string());
					continue;
				}
				parserVersion = mapping->ParserVersion;
				LOG("Build {} (patch {}) -> parser {}", build, mapping->PatchVersion, parserVersion);
			}

			OpcodeCache::Instance().EnsureLoaded(parserVersion);
			if (OpcodeCache::Instance().GetOpcodeCount(parserVersion) == 0)
			{
				LOG("SKIP: No opcodes for parser {}, skipping {}", parserVersion, filePath.string());
				continue;
			}

			if (!versionCache.contains(parserVersion))
			{
				if (!VersionFactory::IsSupported(build))
				{
					LOG("SKIP: Build {} not supported, skipping {}", build, filePath.string());
					continue;
				}
				versionCache.emplace(parserVersion, VersionFactory::Create(build));
			}

			VersionContext& ctx = versionCache.at(parserVersion);
			Stats stats = ParallelProcessor::ProcessAllPackets(reader, ctx.Parser, build, parserVersion, db ? &(*db) : nullptr, 0, toCSV);

			LOG("File done — Parsed: {}, Skipped: {}, Failed: {}, Time: {}ms", stats.ParsedCount, stats.SkippedCount, stats.FailedCount, stats.TotalTime);

			totalStats.ParsedCount += stats.ParsedCount;
            totalStats.SkippedCount += stats.SkippedCount;
            totalStats.FailedCount += stats.FailedCount;
		}
		catch (std::exception const& e)
        {
            LOG("ERROR processing {}: {}", filePath.string(), e.what());
        }
	}

	auto globalEnd = std::chrono::high_resolution_clock::now();
    auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(globalEnd - globalStart).count();

	LOG(">>>>> ALL FILES PARSING COMPLETE <<<<<");
	LOG("Files: {}, Parsed: {}, Skipped: {}, Failed: {}", files.size(), totalStats.ParsedCount, totalStats.SkippedCount, totalStats.FailedCount);
	if (db)
		LOG("DB Stats: {} inserted, {} failed", db->GetTotalInserted(), db->GetTotalFailed());
	LOG("Total time: {}ms ({:.2f} seconds)", totalMs, totalMs / 1000.0);

	curl_global_cleanup();
}