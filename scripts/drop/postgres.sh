#!/bin/bash
set -e
source .env

sudo -u postgres psql -c "DROP DATABASE IF EXISTS $POSTGRES_DB;"
echo "Postgres dropped"