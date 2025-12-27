#!/bin/bash
set -e

echo "=== Checking format ==="
FILES=$(find src tests -name '*.cpp' -o -name '*.h')

for file in $FILES; do
    clang-format --dry-run --Werror "$file"
done