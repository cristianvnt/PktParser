#!/bin/bash
set -e
source .env

cqlsh -e "DROP TABLE IF EXISTS $CASSANDRA_KEYSPACE.packets;"
cqlsh -e "DROP TABLE IF EXISTS $CASSANDRA_KEYSPACE.file_metadata;"

echo "Cassandra dropped"