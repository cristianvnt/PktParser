#!/bin/bash
set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <pkt files directory>"
    exit 1
fi

if [ ! -f tools/sstable/target/sstable-1.0.jar ]; then
    echo ">>> Building SSTable tool <<<"
    (cd tools/sstable && mvn package -q)
fi

PKT_DIR="$1"
CSV_DIR="./csv"
SSTABLE_OUT="./sstable_output"

trap 'curl -s -X PUT "http://localhost:9200/wow_packets/_settings" \
    -H "Content-Type: application/json" -d "{"refresh_interval": "5s"}" > /dev/null' EXIT

curl -s -X PUT "http://localhost:9200/wow_packets/_settings" \
    -H "Content-Type: application/json" -d '{"refresh_interval": "-1"}' > /dev/null

FILE_COUNT=0

time for pkt_file in "$PKT_DIR"/*.pkt; do
    FILE_COUNT=$((FILE_COUNT + 1))
    echo ">>> [$FILE_COUNT] Processing: $(basename "$pkt_file") <<<"

    ./build/PktParser "$pkt_file" --export
    (./utils/run_sstable.sh "$CSV_DIR" "$SSTABLE_OUT" 2>&1 | grep -E "^(Processing|SSTables)")
    sstableloader -d 127.0.0.1 "$SSTABLE_OUT/wow_packets/packets/"

    rm -rf "$CSV_DIR"/*.csv "$SSTABLE_OUT"
done

curl -s -X POST "http://localhost:9200/wow_packets/_forcemerge?max_num_segments=1" > /dev/null

echo ""
echo ">>> DONE - $FILE_COUNT files processed <<<"
