#!/bin/bash
set -e

echo "Building with AddressSanitizer..."

mkdir -p build-debug && cd build-debug

cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer" \
      -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address" ..
make -j4

echo "Done: ./build-debug/PktParser"