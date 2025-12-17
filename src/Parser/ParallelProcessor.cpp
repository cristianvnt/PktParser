#include "pchdef.h"
#include "ParallelProcessor.h"

#include "V11_2_5_64502/JsonSerializer.h"
#include "V11_2_7_64632/JsonSerializer.h"

using namespace PktParser::Reader;
using namespace PktParser::Db;
using namespace PktParser::Versions;

namespace PktParser
{
    void ParallelProcessor::ProcessBatch(std::vector<Pkt> const& batch, size_t startIdx, size_t endIdx,
        VersionContext& ctx, Db::Database& db, std::atomic<size_t>& parsedCount, std::atomic<size_t>& failedCount)
    {
        for (size_t i = startIdx; i < endIdx; ++i)
        {
            Pkt const& pkt = batch[i];
            uint32 pktNumber = pkt.pktNumber;

            ParserMethod method = ctx.Parser->GetParserMethod(pkt.header.opcode);
            if (!method)
                continue;
            
            char const* opcodeName = ctx.Parser->GetOpcodeName(pkt.header.opcode);
            try
            {
                BitReader pktReader = pkt.CreateReader();
                json pktData = method(pktReader);
                json fullPkt = ctx.Serializer->SerializeFullPacket(pkt.header, opcodeName, ctx.Build, pktNumber, pktData);
                db.StorePacket(std::move(fullPkt));
                parsedCount.fetch_add(1, std::memory_order_relaxed);
            }
            catch (std::exception const& e)
            {
                LOG("Failed to parse packet {} OP {}: {}", pktNumber, opcodeName, e.what());
                failedCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    ParallelProcessor::Stats ParallelProcessor::ProcessAllPackets(PktFileReader& reader, VersionContext& ctx, Database& db, size_t threadCount /*= 0*/)
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        if (threadCount == 0)
        {
            threadCount = std::thread::hardware_concurrency();
            if (threadCount == 0)
                threadCount = 4;
        }

        LOG("Using {} threads", threadCount);
        
        std::vector<Pkt> batch;
        batch.reserve(BATCH_SIZE);
        std::atomic<size_t> parsedCount{0};
        std::atomic<size_t> skippedCount{0};
        std::atomic<size_t> failedCount{0};

		while (true)
		{
            std::optional<Pkt> pktOpt = reader.ReadNextPacket();
            if (!pktOpt.has_value())
                break;

			Pkt const& pkt = pktOpt.value();
            if (!ctx.Parser->GetParserMethod(pkt.header.opcode))
            {
                skippedCount.fetch_add(1, std::memory_order_relaxed);
                continue;
            }

            batch.push_back(pkt);

            if (batch.size() >= BATCH_SIZE)
            {
                //LOG("Processing batch of {} packets with {} threads...", batch.size(), threadCount);

                std::vector<std::thread> workers;
                workers.reserve(threadCount);
                
                size_t batchSize = batch.size();
                size_t baseChunkSize = batchSize / threadCount;
                size_t remainder = batchSize % threadCount;

                size_t startIdx = 0;
                for (size_t t = 0; t < threadCount; ++t)
                {
                    size_t chunkSize = baseChunkSize + (t < remainder ? 1 : 0);
                    size_t endIdx = startIdx + chunkSize;

                    if (chunkSize > 0)
                    {
                        workers.emplace_back(ProcessBatch, std::cref(batch), startIdx, endIdx, std::ref(ctx),
                            std::ref(db), std::ref(parsedCount), std::ref(failedCount));
                        startIdx = endIdx;
                    }
                }

                for (auto& worker : workers)
                    worker.join();

                LOG("Batch complete: {} total parsed", parsedCount.load());
                batch.clear();
            }
		}

        if (!batch.empty())
        {
            LOG("Processing final batch of {} packets...", batch.size());

            std::vector<std::thread> workers;
            workers.reserve(threadCount);

            size_t batchSize = batch.size();
            size_t baseChunkSize = batchSize / threadCount;
            size_t remainder = batchSize % threadCount;

            size_t startIdx = 0;
            for (size_t t = 0; t < threadCount; ++t)
            {
                size_t chunkSize = baseChunkSize + (t < remainder ? 1 : 0);
                size_t endIdx = startIdx + chunkSize;

                if (chunkSize > 0)
                {
                    workers.emplace_back(ProcessBatch, std::cref(batch), startIdx, endIdx, std::ref(ctx),
                        std::ref(db), std::ref(parsedCount), std::ref(failedCount));
                    startIdx = endIdx;
                }
            }

            for (auto& worker : workers)
                worker.join();
        }

        auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        return ParallelProcessor::Stats{ parsedCount.load(), skippedCount.load(), failedCount.load(), static_cast<size_t>(duration.count()) };
    }
}