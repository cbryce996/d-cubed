#!/bin/bash
set -e

echo "Starting test script"


TEST_BINARY="build/engine_tests"

if [ ! -f "$TEST_BINARY" ]; then
    echo "❌  Test binary not found at $TEST_BINARY. Make sure build.sh succeeded"
    exit 1
fi

chmod +x "$TEST_BINARY"

echo "Running unit tests"

# Run CTest and capture the result
if ! ctest --test-dir build --output-on-failure; then
    echo "❌  Some tests failed!"
    exit 1
fi

echo "✅  All tests passed!"
