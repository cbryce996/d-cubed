#!/bin/bash
set -e

echo "Starting test script"

TEST_BINARY="build/engine_tests"
COVERAGE_RAW="build/coverage.profraw"
COVERAGE_DATA="build/coverage.profdata"
COVERAGE_HTML_DIR="build/coverage_html"

if [ ! -f "$TEST_BINARY" ]; then
    echo "❌  Test binary not found at $TEST_BINARY. Make sure build.sh succeeded"
    exit 1
fi

chmod +x "$TEST_BINARY"

echo "Running unit tests"

# Normal test run
if ! ctest --test-dir build --output-on-failure; then
    echo "❌  Some tests failed!"
    exit 1
fi

echo "✅  All tests passed!"

# ---------------- COVERAGE ----------------

if [ "$COVERAGE" = "1" ]; then
    echo ""
    echo "📊 Coverage mode enabled"
    echo "Running binary with coverage instrumentation..."

    export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

    rm -f build/coverage_*.profraw

    LLVM_PROFILE_FILE="build/coverage_%p.profraw" "$TEST_BINARY"

    if ! ls build/coverage_*.profraw 1> /dev/null 2>&1; then
        echo "❌ Coverage files not generated!"
        exit 1
    fi

    llvm-profdata merge -sparse build/coverage_*.profraw -o "$COVERAGE_DATA"

    mkdir -p "$COVERAGE_HTML_DIR"

    llvm-cov show "$TEST_BINARY" \
      -instr-profile="$COVERAGE_DATA" \
      --format=html \
      --output-dir="$COVERAGE_HTML_DIR" \
      --ignore-filename-regex="external|tests|/usr/include|/opt/homebrew"

    echo ""
    echo "✅ Coverage report generated!"
    echo "📁 HTML report: $COVERAGE_HTML_DIR/index.html"
fi
