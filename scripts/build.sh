#!/bin/bash
set -e

echo "Starting build script"

BUILD_DIR="build"

rm -rf "$BUILD_DIR"

echo "Configuring CMake"

if [ "$COVERAGE" = "1" ]; then
    echo "📊 Coverage build enabled"
    CMAKE_FLAGS="-DENABLE_COVERAGE=ON"
else
    CMAKE_FLAGS=""
fi

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug $CMAKE_FLAGS

echo "Building game and tests"
cmake --build "$BUILD_DIR"

echo "✅  Build completed successfully!"
