#include "pchdef.h"
#include "ParallelProcessor.h"
#include "Database/OpcodeCache.h"
#include "Misc/Utilities.h"
#include "Common/ParseResult.h"

using namespace PktParser::Reader;
using namespace PktParser::Db;
using namespace PktParser::Versions;
using namespace PktParser::Misc;
using namespace PktParser::Common;

namespace PktParser
{
    ParallelProcessor::ParallelProcessor(Db::Database* db, size_t threadCount /*= 0*/, bool toCSV /*= false*/)
        : _db{ db }, _threadCount{ threadCount }, _toCSV{ toCSV }
    {
        if (toCSV)
            std::filesystem::create_directories("csv");

        if (threadCount == 0)
        {
            _threadCount = std::thread::hardware_concurrency();
            if (_threadCount == 0)
                _threadCount = 4;
        }

        for (size_t i = 0; i < _threadCount; ++i)
            _workers.emplace_back(&ParallelProcessor::WorkerThread, this, i);
    }

    ParallelProcessor::~ParallelProcessor()
    {
        _done.store(true);
        _queueCV.notify_all();

        for (auto& worker : _workers)
            worker.join();
    }

    void ParallelProcessor::ProcessBatch(BatchWork const& work, Db::ElasticClient& es, std::ofstream& csvFile)
    {
        for (Pkt const& pkt : work.Packets)
        {
            char const* opcodeName = OpcodeCache::Instance().GetOpcodeName(work.ParserVersion, pkt.header.opcode);
            try
            {
                BitReader pktReader = pkt.CreateReader();
                std::optional<ParseResult> pktDataOptResult = work.Parser->ParsePacket(pkt.header.opcode, pktReader);
                if (!pktDataOptResult)
                {
                    _skippedCount.fetch_add(1, std::memory_order_relaxed);
                    continue;
                }

                if (_toCSV)
                {
                    std::vector<uint8> compressed;
                    if (pktDataOptResult->storeAsJson)
                        compressed = Misc::CompressJson(pktDataOptResult->json);
                    else
                        compressed = Misc::CompressData(pkt.data);
                    std::string b64 = Misc::Base64Encode(compressed.data(), compressed.size());

                    csvFile << work.Build << ","
                        << work.FileIdStr << ","
                        << (pkt.pktNumber / 10000) << ","
                        << pkt.pktNumber << ","
                        << static_cast<int>(pkt.header.direction) << ","
                        << (pkt.header.packetLength - 4) << ","
                        << pkt.header.opcode << ","
                        << static_cast<int64>(pkt.header.timestamp) << ","
                        << b64 << "\n";

                    es.IndexPacket(pkt.header, opcodeName, work.Build, pkt.pktNumber, *pktDataOptResult, work.SrcFile, work.FileIdStr);
                }
                else
                {
                    es.IndexPacket(pkt.header, opcodeName, work.Build, pkt.pktNumber, *pktDataOptResult, work.SrcFile, work.FileIdStr);
                    if (_db)
                        _db->StorePacket(pkt.header, work.Build, pkt.pktNumber, std::move(pktDataOptResult->json), work.FileId);
                }

                _parsedCount.fetch_add(1, std::memory_order_relaxed);
            }
            catch (std::exception const& e)
            {
                LOG("Failed to parse packet {} OP {}: {}", pkt.pktNumber, opcodeName, e.what());
                _failedCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }

    void ParallelProcessor::WorkerThread(size_t threadNumber)
    {
        static constexpr size_t LOG_EVERY_N_BATCHES = 100;

        ElasticClient es;

        std::ofstream csvFile;
        if (_toCSV)
        {
            std::string path = fmt::format("csv/pkt_thread_{}.csv", threadNumber);
            csvFile.open(path);
        }

        while (true)
        {
            BatchWork work;

            {
                std::unique_lock<std::mutex> lock(_queueMutex);
                _queueCV.wait(lock, [this]{ return !_batchQueue.empty() || _done.load(); });

                if (_batchQueue.empty() && _done.load())
                    break;

                if (!_batchQueue.empty())
                {
                    work = std::move(_batchQueue.front());
                    _batchQueue.pop();
                    _queueCV.notify_all();
                }
            }

            if (!work.Packets.empty())
            {
                ProcessBatch(work, es, csvFile);

                _batchesCompleted.fetch_add(1, std::memory_order_relaxed);
                _completionCV.notify_one();
                
                size_t count = _batchesProcessed.fetch_add(1, std::memory_order_relaxed);
                if (count % LOG_EVERY_N_BATCHES == 0)
                    LOG("Progress: ~{} packets parsed...", _parsedCount.load());
            }
        }

        es.FlushThread();
    }

    ParallelProcessor::Stats ParallelProcessor::ProcessFile(PktFileReader& reader, IVersionParser* parser, uint32 build, std::string const& parserVersion)
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        
        std::string srcFile = reader.GetFilePath();
        CassUuid fileId = Misc::GenerateFileId(reader.GetStartTime(), reader.GetFileSize());

        char uuidStr[CASS_UUID_STRING_LENGTH];
        cass_uuid_string(fileId, uuidStr);
        std::string fileIdStr(uuidStr);
        LOG("Processing file '{}' with UUID {}", srcFile, fileIdStr);

        _parsedCount.store(0);
        _skippedCount.store(0);
        _failedCount.store(0);
        _batchesCompleted.store(0);
        size_t batchesPushed = 0;

        std::vector<Pkt> currentPackets;
        currentPackets.reserve(BATCH_SIZE);

		while (true)
		{
            std::optional<Pkt> pktOpt = reader.ReadNextPacket();
            if (!pktOpt.has_value())
                break;

            currentPackets.push_back(std::move(*pktOpt));

            if (currentPackets.size() >= BATCH_SIZE)
            {
                BatchWork work;
                work.Packets = std::move(currentPackets);
                work.Parser = parser;
                work.Build = build;
                work.ParserVersion = parserVersion;
                work.SrcFile = srcFile;
                work.FileId = fileId;
                work.FileIdStr = fileIdStr;

                {
                    std::unique_lock<std::mutex> lock(_queueMutex);
                    if (!_queueCV.wait_for(lock, std::chrono::seconds(15), [&]{ return _batchQueue.size() < MAX_QED_BATCHES; }))
                        LOG("WARN: Queue full for 15s!");
                    
                    _batchQueue.push(std::move(work));
                    _queueCV.notify_one();
                }

                batchesPushed++;
                currentPackets.clear();
                currentPackets.reserve(BATCH_SIZE);
            }
		}

        if (!currentPackets.empty())
        {
            BatchWork work;
            work.Packets = std::move(currentPackets);
            work.Parser = parser;
            work.Build = build;
            work.ParserVersion = parserVersion;
            work.SrcFile = srcFile;
            work.FileId = fileId;
            work.FileIdStr = fileIdStr;

            std::unique_lock<std::mutex> lock(_queueMutex);
            _batchQueue.push(std::move(work));
            _queueCV.notify_one();
            batchesPushed++;
        }
        
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _completionCV.wait(lock, [this, batchesPushed]{
                return _batchesCompleted.load(std::memory_order_acquire) >= batchesPushed;
            });
        }

        if (_db)
            _db->StoreFileMetadata(fileId, srcFile, build, static_cast<int64>(reader.GetStartTime()), static_cast<uint32>(_parsedCount.load()));

        auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        return Stats{ _parsedCount.load(), _skippedCount.load(), _failedCount.load(), static_cast<size_t>(duration.count()) };
    }
}