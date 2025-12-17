#!/bin/bash

BUILD_TYPE=${1:-RelWithDebInfo}
SYNC_PARSING=${2:-ON}

echo "Building: $BUILD_TYPE, Sync: $SYNC_PARSING"

mkdir -p build && cd build

cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DSYNC_PARSING=$SYNC_PARSING ..
make -j$(nproc)

echo "Done: ./build/PktParser"