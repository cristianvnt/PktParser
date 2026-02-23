#!/bin/bash
set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <PKT_PATH_or_directory> [parser_version]"
    exit 1
fi

if [ ! -f tools/sstable/target/sstable-1.0.jar ]; then
    echo ">>> Building SSTable tool <<<"
    (cd tools/sstable && mvn package -q)
fi

PKT_PATH="$1"
PARSER_VERSION="${2:-}"
CSV_DIR="./csv"
SSTABLE_OUT="./sstable_output"

rm -rf "$CSV_DIR"/*.csv "$SSTABLE_OUT"

cleanup()
{
    curl -s -X PUT "http://localhost:9200/wow_packets/_settings" \
        -H "Content-Type: application/json" -d '{"refresh_interval": "5s"}' > /dev/null
}
trap cleanup EXIT

curl -s -X PUT "http://localhost:9200/wow_packets/_settings" \
    -H "Content-Type: application/json" -d '{"refresh_interval": "-1"}' > /dev/null

echo ">>>>> BULK LOAD PIPELINE <<<<<"
time {
    LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libjemalloc.so.2 ./build/PktParser "$PKT_PATH" --export ${PARSER_VERSION:+--parser-version "$PARSER_VERSION"}

    ./utils/run_sstable.sh "$CSV_DIR" "$SSTABLE_OUT"

    sstableloader -d 127.0.0.1 "$SSTABLE_OUT/wow_packets/packets/"
}

rm -rf "$CSV_DIR"/*.csv "$SSTABLE_OUT"

curl -s -X POST "http://localhost:9200/wow_packets/_forcemerge?max_num_segments=1&wait_for_completion=false" > /dev/null

echo ""
echo ">>> DONE YIPEEEEE <<<"
