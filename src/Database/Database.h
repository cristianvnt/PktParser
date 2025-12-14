#ifndef DATABASE_H
#define DATABASE_H

#include "Reader/PktFileReader.h"
#include "Parser/JsonSerializer.h"

#include <cassandra.h>
#include <vector>
#include <string>
#include <deque>
#include <mutex>
#include <atomic>

namespace PktParser::Db
{
	class Database
	{
	private:
		CassCluster* _cluster;
		CassSession* _session;
		CassPrepared* _preparedInsert;

		size_t _maxPending;
		size_t _batchFlushSize;

		std::deque<CassFuture*> _pendingInserts;
		std::mutex _pendingMutex;

		std::atomic<size_t> _totalInserted{0};
		std::atomic<size_t> _totalFailed{0};
		
		void CreateKeyspaceAndTable();
		void PrepareStmts();

		void CheckOldestInserts(size_t count);

	public:
		Database(size_t maxPending = 10000, size_t batchFlushSize = 1000);
		~Database();

		void StorePacket(json const& pkt);
		void Flush();

		size_t GetTotalInserted() const { return _totalInserted.load(); }
		size_t GetTotalFailed() const { return _totalFailed.load(); }
		size_t GetPendingCount() const
		{
			std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(_pendingMutex));
			return _pendingInserts.size();
		}
	};
}

#endif