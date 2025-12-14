#ifndef PARALLEL_PROCESSOR_H
#define PARALLEL_PROCESSOR_H

#include "Reader/PktFileReader.h"
#include "PktHandler.h"
#include "Database/Database.h"

#include <vector>
#include <thread>
#include <atomic>

namespace PktParser
{
    class ParallelProcessor
    {
    private:
        static constexpr size_t BATCH_SIZE = 5000;
        
        static void ProcessBatch(std::vector<Reader::Pkt> const& batch, size_t startIdx, size_t endIdx, PktRouter& router,
            Db::Database& db, uint32 build, uint32 basePktNumber, std::atomic<size_t>& parsedCount, std::atomic<size_t>& failedCount);
    public:
        struct Stats
        {
            size_t ParsedCount;
            size_t SkippedCount;
            size_t FailedCount;
            size_t TotalTime;
        };

        static Stats ProcessAllPackets(Reader::PktFileReader& reader, PktRouter& router, Db::Database& db, uint32 build, size_t threadCount = 0);
    };
}

#endif