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
	
	class Database
	{
	private:
		struct InsertData
		{
			int packetNumber;
			std::string direction;
			std::string packetName;
			int packetLen;
			std::string opcode;
			std::string timestamp;
			int build;
			std::string pktJson;

			Database* db;
			InsertData* next;
			int retryCount{};
		};

		std::atomic<InsertData*> _poolHead;

		InsertData* AcquireInsertData();
    	void ReleaseInsertData(InsertData* data);

		CassCluster* _cluster;
		CassSession* _session;
		CassPrepared* _preparedInsert;

		std::atomic<size_t> _totalInserted{0};
		std::atomic<size_t> _totalFailed{0};
		std::atomic<size_t> _pendingCount{0};
		
		void PrepareStmts();
		static void InsertCallback(CassFuture* future, void* data);

	public:
		Database();
		~Database();

		CassSession* GetSession() const { return _session; }

		void StorePacket(json&& pkt);

		void Flush();

		size_t GetTotalInserted() const { return _totalInserted.load(); }
		size_t GetTotalFailed() const { return _totalFailed.load(); }
	};
}

#endif