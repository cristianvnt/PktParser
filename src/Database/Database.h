#ifndef DATABASE_H
#define DATABASE_H

#include "Reader/PktFileReader.h"
#include "Parser/JsonSerializer.h"

#include <cassandra.h>
#include <vector>
#include <string>

namespace PktParser::Db
{
	static constexpr size_t BATCH_SIZE = 10;

	class Database
	{
	private:
		CassCluster* _cluster;
		CassSession* _session;
		CassPrepared* _preparedInsert;

		std::vector<json> _batch;
		
		void CreateKeyspaceAndTable();
		void PrepareStmts();

	public:
		Database();
		~Database();

		void StorePacket(json const& pkt);
		void Flush();
	};
}

#endif