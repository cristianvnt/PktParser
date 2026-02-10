#include "pchdef.h"
#include "ElasticClient.h"
#include "Misc/Utilities.h"

using namespace PktParser::Reader;

namespace PktParser::Db
{
    thread_local CURL* ElasticClient::t_curl = nullptr;

    ElasticClient::ElasticClient(std::string const &baseURL /*= "http://localhost:9200"*/)
        : _baseURL{ baseURL }, _documentCount{ 0 }
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ElasticClient::~ElasticClient()
    {
        Flush();
        
        curl_global_cleanup();

        LOG("Elasticsearch shutdown: {} indexed, {} failed", _totalIndexed.load(), _totalFailed.load());
    }

    CURL* ElasticClient::GetCurl()
    {
        if (!t_curl)
        {
            t_curl = curl_easy_init();
            if (!t_curl)
                LOG("WARN: Failed to init curl for thread");
        }
        return t_curl;
    }

    size_t ElasticClient::WriteCallback(char *ptr, size_t size, size_t nmemb, std::string *data)
    {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    }

    void ElasticClient::SendBulk(std::string&& payload, int32 count)
    {
        CURL* curl = GetCurl();
        if (!curl)
        {
            _totalFailed.fetch_add(count, std::memory_order_relaxed);
            return;
        }

        std::string url = _baseURL + "/_bulk";

        // config curl req
        curl_easy_reset(curl);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/x-ndjson");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(headers);

        if (res != CURLE_OK)
        {
            LOG("ES bulk req failed: {}", curl_easy_strerror(res));
            _totalFailed.fetch_add(1, std::memory_order_relaxed);
            return;
        }

        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

        if (httpCode >= 200 && httpCode < 300)
            _totalIndexed.fetch_add(count, std::memory_order_relaxed);
        else
        {
            LOG("ES bulk respose error (HTTP {}): {}", httpCode, response.substr(0, 200));
            _totalFailed.fetch_add(count, std::memory_order_relaxed);
        }
    }
    
    void ElasticClient::IndexPacket(Reader::PktHeader const &header, char const *opcodeName, uint32 build, uint32 pktNumber,
        json const &pktData, std::string const &srcFile, std::string const &fileId)
    {
        bool hasSearchableFields = pktData.contains("SpellID") || pktData.contains("WorldStateId") || pktData.contains("Challenge");
        if (!hasSearchableFields)
            return;

        json doc;
        doc["build"] = build;
        doc["file_id"] = fileId;
        doc["packet_number"] = pktNumber;
        doc["source_file"] = srcFile;
        doc["direction"] = Misc::DirectionToString(header.direction);
        doc["opcode"] = header.opcode;
        doc["packet_name"] = opcodeName;
        doc["timestamp"] = static_cast<int64>(header.timestamp * 1000);

        if (pktData.contains("SpellID"))
            doc["spell_id"] = pktData["SpellID"];

        if (pktData.contains("CastID"))
            doc["cast_id"] = pktData["CastID"].get<std::string>();

        if (pktData.contains("OriginalCastID"))
            doc["original_cast_id"] = pktData["OriginalCastID"].get<std::string>();

        if (pktData.contains("CasterGUID"))
            doc["caster_guid"] = pktData["CasterGUID"].get<std::string>();

        if (pktData.contains("HitTargets"))
        {
            json targetGuids = json::array();
            for (auto const& target : pktData["HitTargets"])
                targetGuids.push_back(target["GUID"].get<std::string>());
            doc["target_guids"] = targetGuids;
        }
        
        if (pktData.contains("HitTargetsCount"))
            doc["hit_count"] = pktData["HitTargetsCount"];

        if (pktData.contains("MissTargetsCount"))
            doc["miss_count"] = pktData["MissTargetsCount"];
        
        std::string actionLine = R"({"index":{"_index":"wow_packets"}})";

        std::string pendingPayload;
        int32 pendingCount = 0;
        {
            std::lock_guard<std::mutex> lock(_bufferMutex);

            _bulkBuffer += actionLine;
            _bulkBuffer += '\n';
            _bulkBuffer += doc.dump();
            _bulkBuffer += '\n';
            _documentCount++;

            if (_documentCount >= BULK_SIZE)
            {
                pendingPayload = std::move(_bulkBuffer);
                pendingCount = _documentCount;
                _bulkBuffer.clear();
                _documentCount = 0;
            }
        }

        if (!pendingPayload.empty())
            SendBulk(std::move(pendingPayload), pendingCount);
    }

    void ElasticClient::Flush()
    {
        std::string pendingPayload;
        int32 pendingCount = 0;

        {
            std::lock_guard<std::mutex> lock(_bufferMutex);

            if (_bulkBuffer.empty())
                return;

            pendingPayload = std::move(_bulkBuffer);
            pendingCount = _documentCount;
            _bulkBuffer.clear();
            _documentCount = 0;
        }

        SendBulk(std::move(pendingPayload), pendingCount);
    }
}