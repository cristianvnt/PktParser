#!/bin/bash
set -e

echo "Building with AddressSanitizer..."

mkdir -p build-debug && cd build-debug

cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1" ..
make -j$(nproc)

echo "Done: ./build-debug/PktParser"