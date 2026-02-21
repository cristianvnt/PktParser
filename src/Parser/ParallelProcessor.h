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
    public:
        struct Stats
        {
            size_t ParsedCount;
            size_t SkippedCount;
            size_t FailedCount;
            size_t TotalTime;
        };

    private:
        static constexpr size_t BATCH_SIZE = 10000;
        static constexpr size_t MAX_QED_BATCHES = 3;

        struct BatchWork
        {
            std::vector<Reader::Pkt> Packets;
            Versions::IVersionParser* Parser;
            uint32 Build;
            std::string ParserVersion;
            std::string SrcFile;
            CassUuid FileId;
            std::string FileIdStr;
        };

        Db::Database* _db;
        std::vector<std::thread> _workers;
        std::queue<BatchWork> _batchQueue;
        std::mutex _queueMutex;
        std::condition_variable _queueCV;
        std::atomic<bool> _done{ false };
        size_t _threadCount;
        bool _toCSV;

        std::atomic<size_t> _parsedCount{ 0 };
        std::atomic<size_t> _skippedCount{ 0 };
        std::atomic<size_t> _failedCount{ 0 };
        std::atomic<size_t> _batchesProcessed{ 0 };
        std::atomic<size_t> _batchesCompleted{ 0 };
	    std::condition_variable _completionCV;
        
        void ProcessBatch(BatchWork const& work, Db::ElasticClient& es, std::ofstream& csvFile);
        void WorkerThread(size_t threadCount);

    public:
        ParallelProcessor(Db::Database* db, size_t threadCount = 0, bool toCSV = false);
        ~ParallelProcessor();

        Stats ProcessFile(Reader::PktFileReader& reader, Versions::IVersionParser* parser, uint32 build, std::string const& parserVersion);
        size_t GetThreadCount() const { return _threadCount; }
    };
}

#endif