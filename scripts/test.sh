#!/bin/bash
set -e

echo "=== Running unit tests ==="
chmod +x build/engine_tests
ctest --test-dir build --output-on-failure