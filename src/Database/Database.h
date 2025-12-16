#ifndef DATABASE_H
#define DATABASE_H

#include "Reader/PktFileReader.h"
#include "Misc/BuildRegistry.h"

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
		CassCluster* _cluster;
		CassSession* _session;
		CassPrepared* _preparedInsert;

		std::deque<std::pair<CassFuture*, size_t>> _pendingInserts;
		std::mutex _pendingMutex;
		size_t _maxPendingInserts;

		std::atomic<size_t> _totalInserted{0};
		std::atomic<size_t> _totalFailed{0};
		
		void CreateKeyspaceAndTable();
		void PrepareStmts();
		void CheckOldestInserts(size_t count);

	public:
		Database(size_t maxPendingInserts = 1000);
		~Database();

		CassSession* GetSession() const { return _session; }

		void StorePacket(json const& pkt);

		void Flush();

		size_t GetTotalInserted() const { return _totalInserted.load(); }
		size_t GetTotalFailed() const { return _totalFailed.load(); }

		// extra
		void InsertBuildMapping(Misc::BuildMappings const& mapping);
	};
}

#endif