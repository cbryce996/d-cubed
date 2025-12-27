#!/bin/bash
set -e

echo "=== Formatting ==="
FILES=$(find src tests -name '*.cpp' -o -name '*.h')

for file in $FILES; do
    clang-format -i "$file"
done