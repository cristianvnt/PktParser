#!/bin/bash
set -e

cqlsh -f scripts/create/cassandra_dev.cql

echo "Cassandra dev ready"