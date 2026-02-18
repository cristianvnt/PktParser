#!/bin/bash
set -e
source .env

read -p "Drop the keyspace as well? (y/n): " choice
choice=${choice:-n}

if [[ "$choice" =~ ^[Yy] ]]; then
    echo "Deleting KEYSPACE $CASSANDRA_KEYSPACE"
    cqlsh -e "DROP KEYSPACE $CASSANDRA_KEYSPACE;"
else
    echo "Just dropping the tables..."
    cqlsh -e "DROP TABLE IF EXISTS $CASSANDRA_KEYSPACE.packets;"
    cqlsh -e "DROP TABLE IF EXISTS $CASSANDRA_KEYSPACE.file_metadata;"
fi

echo "Cassandra dropped"