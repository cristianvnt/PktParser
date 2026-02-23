#!/bin/bash
set -e

BUILD_TYPE="RelWithDebInfo"

for arg in "$@"; do
    case $arg in
        Debug|Release|RelWithDebInfo)
            BUILD_TYPE=$arg
            ;;
        clean )
            echo "Cleaning build directory..."
            rm -rf build
            ;;
    esac
done

echo "Building: $BUILD_TYPE (PARALLEL mode)"

mkdir -p build && cd build
#cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_CXX_FLAGS="-fno-omit-frame-pointer" ..
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
make -j4

echo "Build complete: ./build/PktParser"
