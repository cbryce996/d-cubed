#!/bin/bash
set -e

echo "=== Checking code format ==="
# Exits with error if any files are not correctly formatted
FILES=$(find src tests -regex '.*\.\(cpp\|h\)$')
clang-format -style=file -output-replacements-xml $FILES | grep "<replacement " && \
    echo "❌ Formatting issues found" && exit 1 || echo "✅ Format OK"
