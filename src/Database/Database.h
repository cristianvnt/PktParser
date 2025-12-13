#ifndef DATABASE_H
#define DATABASE_H

#include "Reader/PktFileReader.h"
#include "Parser/JsonSerializer.h"

#include <cassandra.h>
#include <vector>
#include <string>
#include <deque>
#include <mutex>

namespace PktParser::Db
{
	class Database
	{
	private:
		CassCluster* _cluster;
		CassSession* _session;
		CassPrepared* _preparedInsert;

		static constexpr size_t MAX_PENDING = 1000;
		std::deque<CassFuture*> _pendingInserts;
		std::mutex _pendingMutex;

		size_t _totalInserted{};
		size_t _totalFailed{};
		
		void CreateKeyspaceAndTable();
		void PrepareStmts();

		void CheckOldestInserts(size_t count);

	public:
		Database();
		~Database();

		void StorePacket(json const& pkt);
		void Flush();

		size_t GetTotalInserted() const { return _totalInserted; }
		size_t GetTotalFailed() const { return _totalFailed; }
	};
}

#endif