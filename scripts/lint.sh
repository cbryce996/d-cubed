#!/bin/bash
set -e

echo "=== Running clang-tidy lint ==="
# Use -p build to get compile commands
clang-tidy src/**/*.cpp -p build
