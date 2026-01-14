#!/bin/bash
set -e

FILES=$(find src tests \( -name '*.cpp' -o -name '*.h' \))

if [ -z "$FILES" ]; then
    echo "❌  No source files found"
    exit 1
fi

if ! clang-format --dry-run --Werror $FILES; then
    echo "❌  Formatting check failed"
    exit 1
fi

echo "✅  Formatting check passed"
