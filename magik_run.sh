#!/bin/bash
set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <PKT_directory> [ram_gb]"
    echo "-- ram_gb: RAM per tmpfs mount (default: 3, used for both CSV and SSTable)"
    exit 1
fi

PKT_DIR="$1"
RAM_GB="${2:-3}"

CSV_DIR="./csv"
SSTABLE_OUT="./sstable_output"
MAX_BATCH_BYTES=$(( RAM_GB * 1024 * 1024 * 1024 ))
TMPFS_SIZE="${RAM_GB}G"

mkdir -p "$CSV_DIR" "$SSTABLE_OUT"
mountpoint -q "$CSV_DIR" || sudo mount -t tmpfs -o size="$TMPFS_SIZE" tmpfs "$CSV_DIR"
mountpoint -q "$SSTABLE_OUT" || sudo mount -t tmpfs -o size="$TMPFS_SIZE" tmpfs "$SSTABLE_OUT"

cleanup()
{
    curl -s -X PUT "http://localhost:9200/wow_packets/_settings" \
        -H "Content-Type: application/json" -d '{"refresh_interval": "5s"}' > /dev/null

    if [ -n "${SSTABLE_PID:-}" ] && kill -0 "$SSTABLE_PID" 2>/dev/null; then
        echo "QUIT" >&${SSTABLE[1]}
        wait "$SSTABLE_PID" 2>/dev/null
    fi

    mountpoint -q "$CSV_DIR" && sudo umount "$CSV_DIR"
    mountpoint -q "$SSTABLE_OUT" && sudo umount "$SSTABLE_OUT"

    rm -f /tmp/pkt_all_files.txt /tmp/pkt_batch_*
}
trap cleanup EXIT

# wait for a >>SIGNAL<< on the daemon's stdout, ignore everything else
wait_for_signal()
{
    while read -r line <&${SSTABLE[0]}; do
        if [[ "$line" == ">>READY<<" ]]; then
            return 0
        elif [[ "$line" == ">>DONE<<" ]]; then
            return 0
        elif [[ "$line" == ">>ERROR:"* ]]; then
            echo "SSTable error: $line"
            return 1
        fi
    done
    return 1
}

# collect all .pkt files
find "$PKT_DIR" -name "*.pkt" | sort > /tmp/pkt_all_files.txt
TOTAL_FILES=$(wc -l < /tmp/pkt_all_files.txt)

if [ "$TOTAL_FILES" -eq 0 ]; then
    echo "No .pkt files found in $PKT_DIR"
    exit 0
fi

# split into batches by total file size
BATCH_NUM=0
CURRENT_SIZE=0
BATCH_FILE="/tmp/pkt_batch_$(printf '%03d' $BATCH_NUM)"
> "$BATCH_FILE"

while IFS= read -r pkt_file; do
    file_size=$(stat -c%s "$pkt_file")

    if (( CURRENT_SIZE + file_size > MAX_BATCH_BYTES && CURRENT_SIZE > 0 )); then
        BATCH_NUM=$((BATCH_NUM + 1))
        BATCH_FILE="/tmp/pkt_batch_$(printf '%03d' $BATCH_NUM)"
        > "$BATCH_FILE"
        CURRENT_SIZE=0
    fi

    echo "$pkt_file" >> "$BATCH_FILE"
    CURRENT_SIZE=$((CURRENT_SIZE + file_size))
done < /tmp/pkt_all_files.txt

BATCHES=(/tmp/pkt_batch_*)
echo "Found $TOTAL_FILES .pkt files -> ${#BATCHES[@]} batches (max ~${RAM_GB}GB each)"

coproc SSTABLE { ./utils/run_sstable.sh --daemon; }
wait_for_signal
echo "SSTable daemon ready"

curl -s -X PUT "http://localhost:9200/wow_packets/_settings" \
    -H "Content-Type: application/json" -d '{"refresh_interval": "-1"}' > /dev/null

START=$SECONDS
BATCH_NUM=0

for batch_file in "${BATCHES[@]}"; do
    BATCH_NUM=$((BATCH_NUM + 1))
    echo ""
    echo ">>>>> BATCH $BATCH_NUM/${#BATCHES[@]} <<<<<"

    BATCH_DIR=$(mktemp -d)
    while IFS= read -r pkt_file; do
        ln -s "$(realpath "$pkt_file")" "$BATCH_DIR/"
    done < "$batch_file"

    # parse entire batch
    ./build/PktParser "$BATCH_DIR" --export

    rm -rf "$BATCH_DIR"

    # generate SSTables via daemon
    echo "$CSV_DIR $SSTABLE_OUT" >&${SSTABLE[1]}

    if wait_for_signal; then
        # load into Cassandra
        sstableloader -d 127.0.0.1 "$SSTABLE_OUT/wow_packets/packets/"
    else
        echo ">>> Skipping sstableloader for batch $BATCH_NUM (SSTable gen failed)"
    fi

    # flush tmpfs for next batch
    rm -rf "$CSV_DIR"/* "$SSTABLE_OUT"/*

    # compact every 5 batches
    if (( BATCH_NUM % 5 == 0 )); then
        echo ">>> Compacting Cassandra..."
        nodetool compact wow_packets
        sleep 10
    fi
done

echo "QUIT" >&${SSTABLE[1]}
wait "$SSTABLE_PID" 2>/dev/null

ELAPSED=$((SECONDS - START))
echo ""
echo ">>> ALL BATCHES COMPLETE <<<"
echo "Total: $TOTAL_FILES files in ${#BATCHES[@]} batches"
echo "Time: ${ELAPSED}s ($((ELAPSED/60))m $((ELAPSED%60))s)"

curl -s -X POST "http://localhost:9200/wow_packets/_forcemerge?max_num_segments=1&wait_for_completion=false" > /dev/null
nodetool compact wow_packets

echo ">>> DONE YIPEEEEE <<<"