#ifndef PARALLEL_PROCESSOR_H
#define PARALLEL_PROCESSOR_H

#include "Reader/PktFileReader.h"
#include "VersionFactory.h"
#include "Database/Database.h"

#include <vector>
#include <thread>
#include <atomic>

namespace PktParser
{
    using VersionContext = PktParser::Versions::VersionContext;

    class ParallelProcessor
    {
    private:
        static constexpr size_t BATCH_SIZE = 5000;
        
        static void ProcessBatch(std::vector<Reader::Pkt> const& batch, size_t startIdx, size_t endIdx,
            VersionContext& ctx, Db::Database& db, std::atomic<size_t>& parsedCount, std::atomic<size_t>& failedCount);
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