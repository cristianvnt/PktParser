#include "pchdef.h"
#include "ElasticClient.h"
#include "Misc/Utilities.h"

using namespace PktParser::Reader;
using namespace PktParser::Common;

namespace PktParser::Db
{
    thread_local ElasticClient::ThreadContext ElasticClient::t_ctx;

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
            
            t_ctx.headers = nullptr;
            t_ctx.headers = curl_slist_append(t_ctx.headers, "Content-Type: application/x-ndjson");
            t_ctx.buffer.reserve(BULK_RESERVE);
        }
        return t_ctx.curl;
    }

    size_t ElasticClient::WriteCallback(char* ptr, size_t size, size_t nmemb, std::string* data)
    {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    }

    void ElasticClient::BufferDocument(std::string const& docStr, std::string const& fileId, uint32 pktNumber)
    {
        fmt::format_to(std::back_inserter(t_ctx.buffer), R"({{"index":{{"_index":"wow_packets","_id":"{}_{}"}}}})", fileId, pktNumber);
        t_ctx.buffer += '\n';
        t_ctx.buffer.append(docStr);
        t_ctx.buffer += '\n';
        t_ctx.documentCount++;

        if (t_ctx.documentCount >= BULK_SIZE)
        {
            std::string payload = std::move(t_ctx.buffer);
            int32 count = t_ctx.documentCount;
            t_ctx.buffer.clear();
            t_ctx.buffer.reserve(BULK_RESERVE);
            t_ctx.documentCount = 0;

            SendBulk(std::move(payload), count);
        }
    }

    void ElasticClient::WriteBaseDocument(JsonWriter& doc, Reader::PktHeader const& header, char const* opcodeName, 
        uint32 build, uint32 pktNumber, std::string const& srcFile, std::string const& fileId)
    {
        doc.WriteInt("build", build);
        doc.WriteString("file_id", fileId);
        doc.WriteInt("packet_number", pktNumber);
        doc.WriteString("source_file", srcFile);
        doc.WriteString("direction", Misc::DirectionToString(header.direction));
        doc.WriteInt("opcode", header.opcode);
        doc.WriteString("packet_name", opcodeName);
        doc.WriteInt("timestamp", static_cast<int64>(header.timestamp));
    }
        
    void ElasticClient::WriteSpellFields(JsonWriter& doc, Common::SpellSearchFields const& fields)
    {
        doc.WriteInt("spell_id", fields.spellId);
        doc.WriteString("cast_id", fields.castId);
        doc.WriteString("original_cast_id", fields.originalCastId);
        doc.WriteString("caster_guid", fields.casterGuid);
        doc.WriteString("caster_type", fields.casterType);
        doc.WriteUInt("caster_entry", fields.casterEntry);
        doc.WriteUInt("caster_low", fields.casterLow);
        doc.WriteInt("map_id", fields.mapId);
    }
    
    void ElasticClient::IndexPacket(PktHeader const& header, char const* opcodeName, uint32 build, uint32 pktNumber,
        ParseResult const& result, std::string const& srcFile, std::string const& fileId)
    {
        if (!result.spellFields)
            return;


        JsonWriter doc(512);
        doc.BeginObject();
        WriteBaseDocument(doc, header, opcodeName, build, pktNumber, srcFile, fileId);

        if (result.spellFields)
            WriteSpellFields(doc, *result.spellFields);

        doc.EndObject();
        BufferDocument(doc.GetString(), fileId, pktNumber);
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

        for (int attempt = 0; attempt < MAX_RETRIES; ++attempt)
        {
            if (attempt > 0)
            {
                auto delay = std::chrono::seconds(1 << attempt);
                LOG("ES backpressure (429), retry {}/{} in {}s...", attempt, MAX_RETRIES, delay.count());
                std::this_thread::sleep_for(delay);
            }

            // config curl req
            curl_easy_reset(curl);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(payload.size()));
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, t_ctx.headers);

            std::string response;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            if (attempt == 0)
                _totalBytes.fetch_add(payload.size(), std::memory_order_relaxed);

            CURLcode res = curl_easy_perform(curl);

            if (res != CURLE_OK)
            {
                LOG("ES bulk req failed: {}", curl_easy_strerror(res));
                _totalFailed.fetch_add(count, std::memory_order_relaxed);
                return;
            }

            long httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

            if (httpCode >= 200 && httpCode < 300)
            {
                _totalIndexed.fetch_add(count, std::memory_order_relaxed);
                return;
            }

            if (httpCode != 429)
            {
                LOG("ES bulk response error (HTTP {}): {}", httpCode, response.substr(0, 200));
                _totalFailed.fetch_add(count, std::memory_order_relaxed);
                return;
            }
        }

        LOG("ES bulk failed after {} retries", MAX_RETRIES);
        _totalFailed.fetch_add(count, std::memory_order_relaxed);
    }

    void ElasticClient::FlushThread()
    {
        if (!t_ctx.buffer.empty())
        {
            std::string payload = std::move(t_ctx.buffer);
            int32 count = t_ctx.documentCount;
            t_ctx.buffer.clear();
            t_ctx.documentCount = 0;
            SendBulk(std::move(payload), count);
        }

        if (t_ctx.headers)
        {
            curl_slist_free_all(t_ctx.headers);
            t_ctx.headers = nullptr;
        }

        if (t_ctx.curl)
        {
            curl_easy_cleanup(t_ctx.curl);
            t_ctx.curl = nullptr;
        }
    }
}