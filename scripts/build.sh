#!/bin/bash
set -e

echo "Starting build script"

BUILD_DIR="build"

mkdir -p "$BUILD_DIR"

echo "Configuring CMake"
rm -rf build
if ! cmake -B "$BUILD_DIR" -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug; then
    echo "❌  CMake configuration failed!"
    exit 1
fi

echo "Building game and tests"
if ! cmake --build "$BUILD_DIR"; then
    echo "❌  Build failed!"
    exit 1
fi

echo "✅  Build completed successfully!"
