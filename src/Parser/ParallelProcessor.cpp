#include "ParallelProcessor.h"
#include "Misc/Logger.h"
#include "Parser/JsonSerializer.h"
#include "Reader/BitReader.h"
#include "Opcodes.h"

#include <chrono>
#include <algorithm>
#include <nlohmann/json.hpp>

using namespace PktParser::Reader;
using namespace PktParser::Db;

namespace PktParser
{
    void ParallelProcessor::ProcessBatch(std::vector<Pkt> const& batch, size_t startIdx, size_t endIdx, PktRouter& router,
        Db::Database& db, uint32 build, uint32 basePktNumber, std::atomic<size_t>& parsedCount, std::atomic<size_t>& failedCount)
    {
        for (size_t i = startIdx; i < endIdx; ++i)
        {
            Pkt const& pkt = batch[i];
            uint32 pktNumber = pkt.pktNumber;
            try
            {
                BitReader pktReader = pkt.CreateReader();
                json pktData = router.HandlePacket(pkt.header.opcode, pktReader, pktNumber);

                db.StorePacket(JsonSerializer::SerializeFullPacket(pkt.header, build, pktNumber, pktData));
                parsedCount.fetch_add(1, std::memory_order_relaxed);
            }
            catch (std::exception const& e)
            {
                LOG("Failed to parse packet {} OP {}: {}", pktNumber, GetOpcodeName(pkt.header.opcode), e.what());
                LOG("BASE PKT NUMBER: {}", basePktNumber);
                LOG("PKT NUMBER: {}", pktNumber);
                failedCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    ParallelProcessor::Stats ParallelProcessor::ProcessAllPackets(PktFileReader& reader, PktRouter& router,
        Database& db, uint32 build, size_t threadCount /*= 0*/)
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

			if (!router.HasHandler(pkt.header.opcode))
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

                size_t pktsPerThread = batch.size() / threadCount;
                uint32 basePacketNumber = reader.GetPacketNumber();

                for (size_t t = 0; t < threadCount; ++t)
                {
                    size_t startIdx = t * pktsPerThread;
                    size_t endIdx = (t == threadCount - 1) ? batch.size() : startIdx + pktsPerThread;
                    workers.emplace_back(ProcessBatch, std::cref(batch), startIdx, endIdx,
                        std::ref(router), std::ref(db), build, basePacketNumber, std::ref(parsedCount), std::ref(failedCount));
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

            size_t pktsPerThread = batch.size() / threadCount;
            uint32 basePacketNumber = reader.GetPacketNumber();

            for (size_t t = 0; t < threadCount; ++t)
            {
                size_t startIdx = t * pktsPerThread;
                size_t endIdx = (t == threadCount - 1) ? batch.size() : startIdx + pktsPerThread;

                if (startIdx >= batch.size())
                    break;

                workers.emplace_back(ProcessBatch, std::cref(batch), startIdx, endIdx,
                    std::ref(router), std::ref(db), build, basePacketNumber, std::ref(parsedCount), std::ref(failedCount));
            }

            for (auto& worker : workers)
                worker.join();
        }

        auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        return ParallelProcessor::Stats{ parsedCount.load(), skippedCount.load(), failedCount.load(), static_cast<size_t>(duration.count()) };
    }
}