#ifndef PARALLEL_PROCESSOR_H
#define PARALLEL_PROCESSOR_H

#include "Reader/PktFileReader.h"
#include "Database/Database.h"
#include "Database/ElasticClient.h"
#include "IVersionParser.h"

#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <cassandra.h>

namespace PktParser
{
    class ParallelProcessor
    {
    private:
        static constexpr size_t BATCH_SIZE = 10000;
        static constexpr size_t MAX_QED_BATCHES = 3;
        
        static void ProcessBatch(std::vector<Reader::Pkt> const& batch, Versions::IVersionParser* parser, uint32 build, std::string const& parserVersion,
            Db::Database* db, Db::ElasticClient& es, std::string const& srcFile, CassUuid const& fileId, std::string const& fileIdStr,
            std::atomic<size_t>& parsedCount, std::atomic<size_t>& skippedCount, std::atomic<size_t>& failedCount,
            std::ofstream& csvFile, bool toCSV = false);
            
        static void WorkerThread(std::queue<std::vector<Reader::Pkt>>& batchQ, std::mutex& qMutex,
            std::condition_variable& qCV, std::atomic<bool>& done, Versions::IVersionParser* parser, uint32 build, std::string const& parserVersion,
            Db::Database* db, Db::ElasticClient& es, std::string const& srcFile, CassUuid const& fileId, std::string const& fileIdStr,
            std::atomic<size_t>& parsedCount, std::atomic<size_t>& skippedCount, std::atomic<size_t>& failedCount,
            std::atomic<size_t>& batchesProcessed, size_t threadNumber, bool toCSV = false);
    public:
        struct Stats
        {
            size_t ParsedCount;
            size_t SkippedCount;
            size_t FailedCount;
            size_t TotalTime;
        };

        static Stats ProcessAllPackets(Reader::PktFileReader& reader, Versions::IVersionParser* parser, uint32 build, std::string const& parserVersion,
            Db::Database* db, size_t threadCount = 0, bool toCSV = false);
    };
}

#endif