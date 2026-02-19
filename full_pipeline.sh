#!/bin/bash

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <pkt_file> [parser_version]"
    exit 1
fi

if [ ! -f tools/sstable/target/sstable-1.0.jar ]; then
    echo ">>> Building SSTable tool <<<"
    (cd tools/sstable && mvn package -q)
fi

PKT_FILE="$1"
PARSER_VERSION="${2:-}"
CSV_DIR="./csv"
SSTABLE_OUT="./sstable_output"

rm -rf "$CSV_DIR"/*.csv "$SSTABLE_OUT"

echo ">>>>> BULK LOAD PIPELINE <<<<<"
time {
    echo ">>> Export to CSV <<<"
    time ./build/PktParser "$PKT_FILE" --export ${PARSER_VERSION:+--parser-version "$PARSER_VERSION"}

    echo ""
    echo ">>> SSTable Generation <<<"
    time ./utils/run_sstable.sh "$CSV_DIR" "$SSTABLE_OUT"

    echo ""
    echo ">>> SSTable Loading <<<"
    time sstableloader -d 127.0.0.1 "$SSTABLE_OUT/wow_packets/packets/"
}

echo ""
echo ">>> DONE YAY <<<"