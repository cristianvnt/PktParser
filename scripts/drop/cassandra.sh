#!/bin/bash
set -e
source .env

cqlsh -e "DROP KEYSPACE IF EXISTS $CASSANDRA_KEYSPACE;"
echo "Cassandra dropped"