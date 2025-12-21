#!/bin/bash
set -e

DB_NAME="wow_metadata"

echo "This will permanently delete database: $DB_NAME"
echo ""
read -p "Are you sure? ('y' to confirm): " confirmation

if [ "$confirmation" != "y" ]; then
    echo "Aborted"
    exit 0
fi

echo ""
echo "Dropping database: $DB_NAME"
sudo -u postgres psql -c "DROP DATABASE IF EXISTS $DB_NAME;"

if [ $? -eq 0 ]; then
    echo "Database dropped successfully"
    echo ""
    echo "To recreate, run: ./scripts/create_db.sh"
else
    echo "Failed to drop database"
    exit 1
fi