#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

curl -X PUT "http://localhost:9200/wow_packets" \
    -H "Content-Type: application/json" \
    -d @"${SCRIPT_DIR}/elasticsearch_mapping.json"

echo ""
echo "Index created."