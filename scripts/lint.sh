#!/bin/bash
set -e

echo "=== Running clang-tidy lint ==="
clang-tidy src/**/*.cpp -p build
