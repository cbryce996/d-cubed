#!/bin/bash
set -e

echo "=== Generating coverage ==="
lcov --directory build --capture --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --list coverage.info
