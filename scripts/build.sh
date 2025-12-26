#!/bin/bash
set -e

echo "=== Configuring CMake ==="
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug

echo "=== Building game and tests ==="
cmake --build build
