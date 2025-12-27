#!/bin/bash
set -e

echo "Starting format script"

FILES=$(find src tests \( -name '*.cpp' -o -name '*.h' \))

if [ -z "$FILES" ]; then
    echo "❌  No source files found to format!"
    exit 1
fi

for file in $FILES; do
    echo "Formatting $file"
    clang-format -i "$file"
done

echo "✅  Format complete"
