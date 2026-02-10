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
        std::string _baseURL;
        std::string _bulkBuffer;
        int32 _documentCount;
        std::mutex _bufferMutex;

        static constexpr int32 BULK_SIZE = 15000;
        
        std::atomic<size_t> _totalIndexed{ 0 };
        std::atomic<size_t> _totalFailed{ 0 };
        
        static thread_local CURL* t_curl;

        CURL* GetCurl();
        void SendBulk(std::string&& payload, int32 count);
        static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, std::string* data);
    public:
        explicit ElasticClient(std::string const& baseURL = "http://localhost:9200");
        ~ElasticClient();
        
        void IndexPacket(Reader::PktHeader const& header, char const* opcodeName, uint32 build, uint32 pktNumber,
            json const& pktData, std::string const& srcFile, std::string const& fileId);

        void Flush();

        size_t GetTotalIndexed() const { return _totalIndexed.load(); }
        size_t GetTotalFailed() const { return _totalFailed.load(); }
    };
}

#endif