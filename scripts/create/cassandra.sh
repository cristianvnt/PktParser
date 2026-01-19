#!/bin/bash
set -e
source .env

cqlsh -f scripts/create/cassandra.cql

echo "Cassandra ready"