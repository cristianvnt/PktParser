#include "pchdef.h"
#include "ElasticClient.h"
#include "Misc/Utilities.h"

using namespace PktParser::Reader;

namespace PktParser::Db
{
    thread_local ElasticClient::ThreadContext ElasticClient::t_ctx;

    std::unordered_map<std::string_view, ElasticClient::ExtractorFunc> const ElasticClient::_extractors =
    {
        { "SMSG_SPELL_START", &ElasticClient::ExtractSpellFields },
        { "SMSG_SPELL_GO", &ElasticClient::ExtractSpellFields },
    };

    ElasticClient::ElasticClient(std::string const& baseURL /*= "http://localhost:9200"*/)
        : _baseURL{ baseURL }
    {
    }

    ElasticClient::~ElasticClient()
    {
        LOG("Elasticsearch shutdown: {} indexed, {} failed, {:.2f} MB written", _totalIndexed.load(), _totalFailed.load(),
            _totalBytes.load() / (1024.0 * 1024.0));
    }

    CURL* ElasticClient::GetCurl()
    {
        if (!t_ctx.curl)
        {
            t_ctx.curl = curl_easy_init();
            if (!t_ctx.curl)
                LOG("WARN: Failed to init curl for thread");
        }
        return t_ctx.curl;
    }

    size_t ElasticClient::WriteCallback(char* ptr, size_t size, size_t nmemb, std::string* data)
    {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    }

    json ElasticClient::BuildBaseDocument(Reader::PktHeader const& header, char const* opcodeName,
        uint32 build, uint32 pktNumber, std::string const& srcFile, std::string const& fileId)
    {
        json doc;
        doc["build"] = build;
        doc["file_id"] = fileId;
        doc["packet_number"] = pktNumber;
        doc["source_file"] = srcFile;
        doc["direction"] = Misc::DirectionToString(header.direction);
        doc["opcode"] = header.opcode;
        doc["packet_name"] = opcodeName;
        doc["timestamp"] = static_cast<int64>(header.timestamp * 1000);
        return doc;
    }

    bool ElasticClient::ExtractSpellFields(json const& pktData, json& doc)
    {
        if (!pktData.contains("SpellID"))
            return false;

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

        return true;
    }

    void ElasticClient::BufferDocument(json& doc, std::string const& fileId, uint32 pktNumber)
    {
        std::string actionLine = R"({"index":{"_index":"wow_packets","_id":")" + fileId + "_" + std::to_string(pktNumber) + R"("}})";

        t_ctx.buffer += actionLine;
        t_ctx.buffer += '\n';
        t_ctx.buffer += doc.dump();
        t_ctx.buffer += '\n';
        t_ctx.documentCount++;

        if (t_ctx.documentCount >= BULK_SIZE)
        {
            std::string payload = std::move(t_ctx.buffer);
            int32 count = t_ctx.documentCount;
            t_ctx.buffer.clear();
            t_ctx.documentCount = 0;

            SendBulk(std::move(payload), count);
        }
    }
    
    void ElasticClient::IndexPacket(Reader::PktHeader const& header, char const* opcodeName, uint32 build, uint32 pktNumber,
        json const& pktData, std::string const& srcFile, std::string const& fileId)
    {
        auto it = _extractors.find(opcodeName);
        if (it == _extractors.end())
            return;

        json doc = BuildBaseDocument(header, opcodeName, build, pktNumber, srcFile, fileId);

        if (!it->second(pktData, doc))
            return;

        BufferDocument(doc, fileId, pktNumber);
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

        _totalBytes.fetch_add(payload.size(), std::memory_order_relaxed);
        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(headers);

        if (res != CURLE_OK)
        {
            LOG("ES bulk req failed: {}", curl_easy_strerror(res));
            _totalFailed.fetch_add(count, std::memory_order_relaxed);
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

    void ElasticClient::FlushThread()
    {
        if (t_ctx.buffer.empty())
            return;

        std::string payload = std::move(t_ctx.buffer);
        int32 count = t_ctx.documentCount;
        t_ctx.buffer.clear();
        t_ctx.documentCount = 0;

        SendBulk(std::move(payload), count);

        if (t_ctx.curl)
        {
            curl_easy_cleanup(t_ctx.curl);
            t_ctx.curl = nullptr;
        }
    }
}