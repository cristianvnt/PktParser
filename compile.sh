#!/bin/bash
set -e

BUILD_TYPE="RelWithDebInfo"
CLEAN=false

for arg in "$@"; do
    case $arg in
        Debug|Release|RelWithDebInfo)
            BUILD_TYPE=$arg
            ;;
        clean)
            CLEAN=true
            ;;
    esac
done

if $CLEAN; then
    echo "Cleaning build directory..."
    rm -rf build
fi

echo "Building: $BUILD_TYPE"

CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
if [ -n "$VCPKG_ROOT" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
fi

mkdir -p build && cd build
cmake $CMAKE_ARGS ..
make -j4

echo "Build complete: ./build/PktParser"