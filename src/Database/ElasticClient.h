#ifndef ELASTIC_CLIENT_H
#define ELASTIC_CLIENT_H

#include "Misc/Define.h"
#include "Reader/PktFileReader.h"
#include "Common/ParseResult.h"
#include "Common/JsonWriter.h"

#include <string>
#include <mutex>
#include <atomic>
#include <curl/curl.h>

namespace PktParser::Db
{
    class ElasticClient
    {
    private:
        static constexpr int32 BULK_SIZE = 15000;
        static constexpr int32 MAX_RETRIES = 3;
        static constexpr size_t BULK_RESERVE = BULK_SIZE * 500;
        std::string _baseURL;
        
        std::atomic<size_t> _totalIndexed{ 0 };
        std::atomic<size_t> _totalFailed{ 0 };

        std::atomic<size_t> _totalBytes{ 0 };
        
        struct ThreadContext
        {
            CURL* curl = nullptr;
            curl_slist* headers = nullptr;
            std::string buffer;
            int32 documentCount = 0;
        };
        static thread_local ThreadContext t_ctx;

        CURL* GetCurl();
        void SendBulk(std::string&& payload, int32 count);
        static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, std::string* data);

        void BufferDocument(std::string const& docStr, std::string const& fileId, uint32 pktNumber);
        static void WriteBaseDocument(Common::JsonWriter& doc, Reader::PktHeader const& header, char const* opcodeName,
            uint32 build, uint32 pktNumber, std::string const& srcFile, std::string const& fileId);
        
        static void WriteSpellFields(Common::JsonWriter& doc, Common::SpellSearchFields const& fields);
        
    public:
        explicit ElasticClient(std::string const& baseURL = "http://localhost:9200");
        ~ElasticClient();
        
        void IndexPacket(Reader::PktHeader const& header, char const* opcodeName, uint32 build, uint32 pktNumber,
            Common::ParseResult const& result, std::string const& srcFile, std::string const& fileId);

        void FlushThread();

        size_t GetTotalIndexed() const { return _totalIndexed.load(); }
        size_t GetTotalFailed() const { return _totalFailed.load(); }
        size_t GetTotalBytes() const { return _totalBytes.load(); }
        int32 GetMaxBulk() const { return BULK_SIZE; }
    };
}

#endif