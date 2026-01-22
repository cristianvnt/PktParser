#ifndef DATABASE_H
#define DATABASE_H

#include "Reader/PktFileReader.h"

#include <nlohmann/json.hpp>
#include <cassandra.h>
#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include <atomic>
#include <utility>

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
		int packetNumber;
		std::string direction;
		std::string packetName;
		int packetLen;
		std::string opcode;
		std::string timestamp;
		int build;
		std::string sourceFile;
		CassUuid fileId;
		std::string pktJson;

		CallbackContext* context;

		CassFuture* future;
		InsertData* next;
		int retryCount{};
	};
	
	class Database
	{
	private:
		std::atomic<InsertData*> _poolHead;
		CassCluster* _cluster;
		CassSession* _session;
		CassPrepared* _preparedInsert;
		CassUuidGen* _uuidGen;

		std::atomic<size_t> _totalInserted{0};
		std::atomic<size_t> _totalFailed{0};
		std::atomic<size_t> _pendingCount{0};

		CallbackContext _callbackContext;

		InsertData* AcquireInsertData();
		void PrepareStmts();

		static void InsertCallback(CassFuture* future, void* data);
		static void BindInsertStatement(CassStatement* stmt, InsertData const* data);

	public:
		Database();
		~Database();

		CassSession* GetSession() const { return _session; }

		void StorePacket(json&& pkt, std::string const& srcFile, CassUuid const& fileId);
		void Flush();

		CassUuid GenerateFileId();

		size_t GetTotalInserted() const { return _totalInserted.load(); }
		size_t GetTotalFailed() const { return _totalFailed.load(); }
	};
}

#endif