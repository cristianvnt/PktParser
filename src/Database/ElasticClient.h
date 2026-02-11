#ifndef ELASTIC_CLIENT_H
#define ELASTIC_CLIENT_H

#include "Misc/Define.h"
#include "Reader/PktFileReader.h"

#include <string>
#include <mutex>
#include <atomic>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace PktParser::Db
{
    using json = nlohmann::ordered_json;

    class ElasticClient
    {
    private:
        static constexpr int32 BULK_SIZE = 5000;
        std::string _baseURL;
        
        std::atomic<size_t> _totalIndexed{ 0 };
        std::atomic<size_t> _totalFailed{ 0 };

        std::atomic<size_t> _totalBytes{ 0 };
        
        struct ThreadContext
        {
            CURL* curl = nullptr;
            std::string buffer;
            int32 documentCount = 0;
        };
        static thread_local ThreadContext t_ctx;

        using ExtractorFunc = bool(*)(json const&, json&);
        static std::unordered_map<std::string_view, ExtractorFunc> const _extractors;

        CURL* GetCurl();
        void SendBulk(std::string&& payload, int32 count);
        static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, std::string* data);

        static json BuildBaseDocument(Reader::PktHeader const& header, char const* opcodeName,
            uint32 build, uint32 pktNumber, std::string const& srcFile, std::string const& fileId);
        
        // extractors
        static bool ExtractSpellFields(json const& pktData, json& doc);
        
        void BufferDocument(json& doc, std::string const& fileId, uint32 pktNumber);
    public:
        explicit ElasticClient(std::string const& baseURL = "http://localhost:9200");
        ~ElasticClient();
        
        void IndexPacket(Reader::PktHeader const& header, char const* opcodeName, uint32 build, uint32 pktNumber,
            json const& pktData, std::string const& srcFile, std::string const& fileId);

        void FlushThread();

        size_t GetTotalIndexed() const { return _totalIndexed.load(); }
        size_t GetTotalFailed() const { return _totalFailed.load(); }
        size_t GetTotalBytes() const { return _totalBytes.load(); }
    };
}

#endif