#!/bin/bash
set -e

DB_NAME="wow_metadata"
DB_USER="wowparser"
DB_HOST="localhost"
DB_PASS="wowparser123"

echo "Creating Postgres user if not exists..."
sudo -u postgres psql << EOF
DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = '$DB_USER') THEN
        CREATE USER $DB_USER WITH PASSWORD '$DB_PASS';
        ALTER USER $DB_USER CREATEDB;
        RAISE NOTICE 'User $DB_USER created';
    ELSE
        RAISE NOTICE 'User $DB_USER already exists';
    END IF;
END
\$\$;
\q
EOF

echo ""
echo "Checking if database exists..."

if sudo -u postgres psql -lqt | cut -d \| -f 1 | grep -qw "$DB_NAME"; then
    echo ""
    echo "Error: Database '$DB_NAME' already exists"
    echo ""
    echo "To recreate the database, manually drop it first:"
    echo "  sudo -u postgres psql -c \"DROP DATABASE $DB_NAME;\""
    echo ""
    echo "Then run this script again."
    exit 1
fi

echo "Creating database: $DB_NAME"
sudo -u postgres psql -c "CREATE DATABASE $DB_NAME OWNER $DB_USER;"

if [ $? -eq 0 ]; then
    echo "Database '$DB_NAME' created successfully"
else
    echo "Failed to create database"
    exit 1
fi

echo ""

echo "Creating tables..."
psql -U $DB_USER -d $DB_NAME -h $DB_HOST < scripts/create_tables.sql

if [ $? -eq 0 ]; then
    echo "Tables created successfully"
    echo ""
    echo "Database '$DB_NAME' is ready"
    echo "Credentials: $DB_USER / $DB_PASS"
else
    echo ""
    echo "Error: Failed to create tables"
    exit 1
fi