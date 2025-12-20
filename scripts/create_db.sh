#!/bin/bash

DB_NAME="wow_metadata"
DB_USER="wowparser"
DB_HOST="localhost"

echo "Creating database: $DB_NAME"
sudo -u postgres psql -c "DROP DATABASE IF EXISTS $DB_NAME;" 2>/dev/null
sudo -u postgres psql -c "CREATE DATABASE $DB_NAME OWNER $DB_USER;"

if [ $? -eq 0 ]; then
    echo "Database '$DB_NAME' created successfully"
else
    echo "Database '$DB_NAME' already exists (skipping creation)"
fi

echo ""

echo "Creating tables..."
psql -U $DB_USER -d $DB_NAME -h $DB_HOST < scripts/create_tables.sql

if [ $? -eq 0 ]; then
    echo "Tables created successfully"
    echo "Database '$DB_NAME' is READY"
else
    echo ""
    echo "Error: Failed to create tables"
    exit 1
fi