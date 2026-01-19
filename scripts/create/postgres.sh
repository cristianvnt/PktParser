#!/bin/bash
set -e
source .env

sudo -u postgres psql -c "CREATE USER $POSTGRES_USER WITH PASSWORD '$POSTGRES_PASSWORD' CREATEDB;" 2>/dev/null || true
sudo -u postgres psql -c "CREATE DATABASE $POSTGRES_DB OWNER $POSTGRES_USER;"
psql -U $POSTGRES_USER -d $POSTGRES_DB -h $POSTGRES_HOST < scripts/create/postgres.sql

echo "Postgres DB ready"