#ifndef PARALLEL_PROCESSOR_H
#define PARALLEL_PROCESSOR_H

#include "Reader/PktFileReader.h"
#include "VersionFactory.h"
#include "Database/Database.h"

#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <cassandra.h>

namespace PktParser
{
    using VersionContext = PktParser::Versions::VersionContext;

    class ParallelProcessor
    {
    private:
        static constexpr size_t BATCH_SIZE = 10000;
        static constexpr size_t MAX_QED_BATCHES = 3;
        
        static void ProcessBatch(std::vector<Reader::Pkt> const& batch, VersionContext& ctx,
            Db::Database& db, std::string const& srcFile, CassUuid const& fileId,
            std::atomic<size_t>& parsedCount, std::atomic<size_t>& skippedCount, std::atomic<size_t>& failedCount);
            
        static void WorkerThread(std::queue<std::vector<Reader::Pkt>>& batchQ, std::mutex& qMutex,
            std::condition_variable& qCV, std::atomic<bool>& done,
            VersionContext& ctx, Db::Database& db, std::string const& srcFile, CassUuid const& fileId,
            std::atomic<size_t>& parsedCount, std::atomic<size_t>& skippedCount, std::atomic<size_t>& failedCount,
            std::atomic<size_t>& batchesProcessed);
    public:
        struct Stats
        {
            size_t ParsedCount;
            size_t SkippedCount;
            size_t FailedCount;
            size_t TotalTime;
        };

        static Stats ProcessAllPackets(Reader::PktFileReader& reader, VersionContext& ctx, Db::Database& db, size_t threadCount = 0);
    };
}

#endif