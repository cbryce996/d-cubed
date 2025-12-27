#!/bin/bash
set -e

echo "=== Running unit tests ==="
ctest --test-dir build --output-on-failure