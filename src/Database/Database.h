#ifndef DATABASE_H
#define DATABASE_H

#include "Reader/PktFileReader.h"

#include <nlohmann/json.hpp>
#include <cassandra.h>
#include <string>
#include <atomic>

namespace PktParser::Db
{
	using json = nlohmann::ordered_json;

	class Database;
	struct InsertData;

	struct CallbackContext
	{
		std::atomic<size_t>* totalInserted;
        std::atomic<size_t>* totalFailed;
        std::atomic<size_t>* pendingCount;
        std::atomic<InsertData*>* poolHead;
        CassPrepared* preparedStmt;
		CassSession* session;

		void ReleaseToPool(InsertData* data);
	};

	struct InsertData
	{
		int32 build;
		CassUuid fileId;
		int32 bucket;
		int32 packetNumber;
		uint8 direction;
		int32 packetLen;
		int32 opcode;
		int64 timestamp;
		std::vector<uint8> compressedJson;

		CallbackContext* context;

		InsertData* next;
		int32 retryCount{};
	};
	
	class Database
	{
	private:
		std::atomic<InsertData*> _poolHead;
		CassCluster* _cluster;
		CassSession* _session;
		CassPrepared* _preparedInsert;
		CassPrepared* _preparedMetadata;

		std::atomic<size_t> _totalInserted{ 0 };
		std::atomic<size_t> _totalFailed{ 0 };
		std::atomic<size_t> _pendingCount{ 0 };
		std::atomic<size_t> _totalBytes{ 0 };
		std::atomic<size_t> _totalCompressedBytes{ 0 };

		static constexpr size_t MAX_PENDING = 8192;

		CallbackContext _callbackContext;

		InsertData* AcquireInsertData();
		void PrepareStmts();

		static void InsertCallback(CassFuture* future, void* data);
		static void BindInsertStatement(CassStatement* stmt, InsertData const* data);
		static void RetryInsert(InsertData* data);
		static std::vector<uint8> CompressJson(std::string const& json);

	public:
		Database();
		~Database();

		CassSession* GetSession() const { return _session; }

		void StoreFileMetadata(CassUuid const& fileId, std::string const& srcFile, uint32 build, int64 startTime, uint32 pktCount);
		void StorePacket(Reader::PktHeader const& header, uint32 build, uint32 pktNumber, json&& pktData, CassUuid const& fileId);

		void Flush();

		CassUuid GenerateFileId(uint32 startTime, size_t fileSize);

		size_t GetTotalInserted() const { return _totalInserted.load(); }
		size_t GetTotalFailed() const { return _totalFailed.load(); }
		size_t GetTotalBytes() const { return _totalBytes.load(); }
	};
}

#endif