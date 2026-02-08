#include "pchdef.h"
#include "ParallelProcessor.h"

#include "V11_2_5_63506/JsonSerializer.h"
#include "V11_2_7_64632/JsonSerializer.h"

using namespace PktParser::Reader;
using namespace PktParser::Db;
using namespace PktParser::Versions;

namespace PktParser
{
    void ParallelProcessor::ProcessBatch(std::vector<Pkt> const& batch, VersionContext& ctx,
        Db::Database& db, std::string const& srcFile, CassUuid const& fileId,
        std::atomic<size_t>& parsedCount, std::atomic<size_t>& skippedCount, std::atomic<size_t>& failedCount)
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

                db.StorePacket(pkt.header, opcodeName, ctx.Build, pkt.pktNumber, std::move(*pktDataOpt), srcFile, fileId);

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
        VersionContext ctx, Db::Database& db, std::string const& srcFile, CassUuid const& fileId,
        std::atomic<size_t>& parsedCount, std::atomic<size_t>& skippedCount, std::atomic<size_t>& failedCount,
        std::atomic<size_t>& batchesProcessed)
    {
        static constexpr size_t LOG_EVERY_N_BATCHES = 100;

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
                ProcessBatch(batch, ctx, db, srcFile, fileId, parsedCount, skippedCount, failedCount);
                
                size_t count = batchesProcessed.fetch_add(1, std::memory_order_relaxed);
                if (count % LOG_EVERY_N_BATCHES == 0)
                    LOG("Progress: ~{} packets parsed...", parsedCount.load());
            }
        }
    }

    ParallelProcessor::Stats ParallelProcessor::ProcessAllPackets(PktFileReader& reader, Database& db, size_t threadCount /*= 0*/)
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
        CassUuid fileId = db.GenerateFileId();

        char uuidStr[CASS_UUID_STRING_LENGTH];
        cass_uuid_string(fileId, uuidStr);
        LOG("Processing file '{}' with UUID {}", srcFile, uuidStr);
        
        std::queue<std::vector<Pkt>> batchQueue;
        std::mutex queueMutex;
        std::condition_variable queueCV;
        std::atomic<bool> done{ false };

        std::atomic<size_t> parsedCount{0};
        std::atomic<size_t> skippedCount{0};
        std::atomic<size_t> failedCount{0};
        std::atomic<size_t> batchesProcessed{0};

        std::vector<std::thread> workers;
        workers.reserve(threadCount);
        for (size_t i = 0; i < threadCount; ++i)
        {
            VersionContext ctx = VersionFactory::Create(reader.GetBuildVersion());
            workers.emplace_back(WorkerThread, std::ref(batchQueue), std::ref(queueMutex), std::ref(queueCV), std::ref(done),
                std::move(ctx), std::ref(db), std::ref(srcFile), std::ref(fileId), std::ref(parsedCount), std::ref(skippedCount), std::ref(failedCount),
                std::ref(batchesProcessed));
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

        auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        return ParallelProcessor::Stats{ parsedCount.load(), skippedCount.load(), failedCount.load(), static_cast<size_t>(duration.count()) };
    }
}