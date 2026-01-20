#!/bin/bash
set -e

BUILD_TYPE="RelWithDebInfo"
SYNC_MODE="OFF"

for arg in "$@"; do
    case $arg in
        sync )
            SYNC_MODE="ON"
            ;;
        Debug|Release|RelWithDebInfo)
            BUILD_TYPE=$arg
            ;;
    esac
done

MODE_TEXT=$( [ "$SYNC_MODE" = "ON" ] && echo "SYNC" || echo "PARALLEL" )
echo "Building: $BUILD_TYPE ($MODE_TEXT mode)"

mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DSYNC_PARSING=$SYNC_MODE ..
make -j$(nproc)

echo "Build complete: ./build/PktParser"