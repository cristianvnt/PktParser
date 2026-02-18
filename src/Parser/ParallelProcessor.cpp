#include "pchdef.h"
#include "ParallelProcessor.h"

using namespace PktParser::Reader;
using namespace PktParser::Db;
using namespace PktParser::Versions;
using namespace PktParser::Misc;

namespace PktParser
{
    void ParallelProcessor::ProcessBatch(std::vector<Pkt> const& batch, VersionContext& ctx,
        Db::Database& db, Db::ElasticClient& es, std::string const& srcFile, CassUuid const& fileId, std::string const& fileIdStr,
        std::atomic<size_t>& parsedCount, std::atomic<size_t>& skippedCount, std::atomic<size_t>& failedCount,
        std::ofstream& csvFile, bool toCSV /*= false*/)
    {
        for (Pkt const& pkt : batch)
        {
            char const* opcodeName = ctx.Parser->GetOpcodeName(pkt.header.opcode);
            try
            {
                BitReader pktReader = pkt.CreateReader();
                std::optional<json> pktDataOpt = ctx.Parser->ParsePacket(pkt.header.opcode, pktReader);
                if (!pktDataOpt)
                {
                    skippedCount.fetch_add(1, std::memory_order_relaxed);
                    continue;
                }

                if (toCSV)
                {
                    std::string jsonStr = pktDataOpt->dump();
                    std::vector<uint8> compressed = Database::CompressJson(jsonStr);
                    std::string b64 = Base64Encode(compressed.data(), compressed.size());

                    csvFile << ctx.Build << ","
                        << fileIdStr << ","
                        << (pkt.pktNumber / 10000) << ","
                        << pkt.pktNumber << ","
                        << static_cast<int>(pkt.header.direction) << ","
                        << (pkt.header.packetLength - 4) << ","
                        << pkt.header.opcode << ","
                        << static_cast<int64_t>(pkt.header.timestamp * 1000) << ","
                        << b64 << "\n";
                }
                else
                {
                    es.IndexPacket(pkt.header, opcodeName, ctx.Build, pkt.pktNumber, *pktDataOpt, srcFile, fileIdStr);
                    db.StorePacket(pkt.header, ctx.Build, pkt.pktNumber, std::move(*pktDataOpt), fileId);
                }

                parsedCount.fetch_add(1, std::memory_order_relaxed);
            }
            catch (std::exception const& e)
            {
                LOG("Failed to parse packet {} OP {}: {}", pkt.pktNumber, opcodeName, e.what());
                failedCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    void ParallelProcessor::WorkerThread(std::queue<std::vector<Reader::Pkt>>& batchQ, std::mutex& qMutex, std::condition_variable& qCV, std::atomic<bool>& done,
        VersionContext ctx, Db::Database& db, Db::ElasticClient& es,std::string const& srcFile, CassUuid const& fileId, std::string const& fileIdStr,
        std::atomic<size_t>& parsedCount, std::atomic<size_t>& skippedCount, std::atomic<size_t>& failedCount,
        std::atomic<size_t>& batchesProcessed, size_t threadNumber, bool toCSV /*= false*/)
    {
        static constexpr size_t LOG_EVERY_N_BATCHES = 100;

        std::ofstream csvFile;
        if (toCSV)
        {
            std::string path = fmt::format("csv/pkt_out_{}.csv", threadNumber);
            csvFile.open(path);
        }

        while (true)
        {
            std::vector<Pkt> batch;

            {
                std::unique_lock<std::mutex> lock(qMutex);
                qCV.wait(lock, [&]{ return !batchQ.empty() || done.load(); });

                if (batchQ.empty() && done.load())
                    break;

                if (!batchQ.empty())
                {
                    batch = std::move(batchQ.front());
                    batchQ.pop();
                    qCV.notify_all();
                }
            }

            if (!batch.empty())
            {
                ProcessBatch(batch, ctx, db, es, srcFile, fileId, fileIdStr, parsedCount, skippedCount, failedCount, csvFile, toCSV);
                
                size_t count = batchesProcessed.fetch_add(1, std::memory_order_relaxed);
                if (count % LOG_EVERY_N_BATCHES == 0)
                    LOG("Progress: ~{} packets parsed...", parsedCount.load());
            }
        }

        es.FlushThread();
    }

    ParallelProcessor::Stats ParallelProcessor::ProcessAllPackets(PktFileReader& reader, Database& db, size_t threadCount /*= 0*/, bool toCSV /*= false*/)
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        if (threadCount == 0)
        {
            threadCount = std::thread::hardware_concurrency();
            if (threadCount == 0)
                threadCount = 4;
        }

        LOG("Using {} threads", threadCount);

        std::string srcFile = reader.GetFilePath();
        CassUuid fileId = db.GenerateFileId(reader.GetStartTime(), reader.GetFileSize());

        char uuidStr[CASS_UUID_STRING_LENGTH];
        cass_uuid_string(fileId, uuidStr);
        std::string fileIdStr(uuidStr);
        LOG("Processing file '{}' with UUID {}", srcFile, fileIdStr);

        ElasticClient es;
        
        std::queue<std::vector<Pkt>> batchQueue;
        std::mutex queueMutex;
        std::condition_variable queueCV;
        std::atomic<bool> done{ false };

        std::atomic<size_t> parsedCount{0};
        std::atomic<size_t> skippedCount{0};
        std::atomic<size_t> failedCount{0};
        std::atomic<size_t> batchesProcessed{0};

        if (toCSV)
            std::filesystem::create_directories("csv");

        std::vector<std::thread> workers;
        workers.reserve(threadCount);
        for (size_t i = 0; i < threadCount; ++i)
        {
            VersionContext ctx = VersionFactory::Create(reader.GetBuildVersion());
            workers.emplace_back(WorkerThread, std::ref(batchQueue), std::ref(queueMutex), std::ref(queueCV), std::ref(done),
                std::move(ctx), std::ref(db), std::ref(es), std::ref(srcFile), std::ref(fileId), std::ref(fileIdStr),
                std::ref(parsedCount), std::ref(skippedCount), std::ref(failedCount),
                std::ref(batchesProcessed), i, toCSV);
        }

        std::vector<Pkt> currentBatch;
        currentBatch.reserve(BATCH_SIZE);

		while (true)
		{
            std::optional<Pkt> pktOpt = reader.ReadNextPacket();
            if (!pktOpt.has_value())
                break;

            currentBatch.push_back(std::move(*pktOpt));

            if (currentBatch.size() >= BATCH_SIZE)
            {
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    if (!queueCV.wait_for(lock, std::chrono::seconds(15), [&]{ return batchQueue.size() < MAX_QED_BATCHES; }))
                        LOG("WARN: Queue full for 15s!");
                    
                    batchQueue.push(std::move(currentBatch));
                    queueCV.notify_one();
                }

                currentBatch.clear();
                currentBatch.reserve(BATCH_SIZE);
            }
		}

        if (!currentBatch.empty())
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            batchQueue.push(std::move(currentBatch));
            queueCV.notify_one();
        }

        done.store(true);
        queueCV.notify_all();

        for (auto& worker : workers)
            worker.join();

        db.StoreFileMetadata(fileId, srcFile, reader.GetBuildVersion(), static_cast<int64>(reader.GetStartTime()), static_cast<uint32>(parsedCount));

        LOG("ES Stats: {} indexed, {} failed", es.GetTotalIndexed(), es.GetTotalFailed());
        auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        double seconds = duration.count() / 1000.0;
        double dbMB = db.GetTotalBytes() / (1024.0 * 1024.0);
        double esMB = es.GetTotalBytes() / (1024.0 * 1024.0);
        LOG("Cassandra: {:.2f} MB ({:.2f} MB/s)", dbMB, dbMB / seconds);
        LOG("ES: {:.2f} MB ({:.2f} MB/s)", esMB, esMB / seconds);

        return ParallelProcessor::Stats{ parsedCount.load(), skippedCount.load(), failedCount.load(), static_cast<size_t>(duration.count()) };
    }
}