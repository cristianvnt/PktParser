#ifndef DATABASE_H
#define DATABASE_H

#include "Reader/PktFileReader.h"

#include <nlohmann/json.hpp>
#include <cassandra.h>
#include <string>
#include <atomic>
#include <vector>
#include <mutex>

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
		std::mutex* poolMutex;
        std::vector<InsertData*>* pool;
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
		int32 retryCount{};
	};
	
	class Database
	{
	private:
		std::mutex _poolMutex;
		std::vector<InsertData*> _pool;
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
		
	public:
		Database();
		~Database();
		
		CassSession* GetSession() const { return _session; }
		
		void StoreFileMetadata(CassUuid const& fileId, std::string const& srcFile, uint32 build, int64 startTime, uint32 pktCount);
		void StorePacket(Reader::PktHeader const& header, uint32 build, uint32 pktNumber, json&& pktData, CassUuid const& fileId);
		
		void Flush();

		size_t GetTotalInserted() const { return _totalInserted.load(); }
		size_t GetTotalFailed() const { return _totalFailed.load(); }
		size_t GetTotalBytes() const { return _totalBytes.load(); }
	};
}

#endif