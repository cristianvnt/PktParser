#!/bin/bash
set -e
source .env

cqlsh -f scripts/create/cassandra_dev.cql

echo "Cassandra dev ready"