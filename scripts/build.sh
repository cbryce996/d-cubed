#!/bin/bash
set -euo pipefail

echo "Starting build script"

BUILD_DIR="build"
rm -rf "$BUILD_DIR"

if [ -z "${VCPKG_ROOT:-}" ]; then
  echo "❌ VCPKG_ROOT is not set."
  echo "Set it, e.g. in ~/.zshrc:  export VCPKG_ROOT=\"$HOME/vcpkg\""
  exit 1
fi

TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
if [ ! -f "$TOOLCHAIN_FILE" ]; then
  echo "❌ vcpkg toolchain file not found at: $TOOLCHAIN_FILE"
  echo "Check that VCPKG_ROOT points to a vcpkg git checkout."
  exit 1
fi

echo "Using vcpkg toolchain: $TOOLCHAIN_FILE"

CMAKE_FLAGS=(
  "-DCMAKE_BUILD_TYPE=Debug"
  "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_FILE"
)

if [ "${COVERAGE:-0}" = "1" ]; then
  echo "📊 Coverage build enabled"
  CMAKE_FLAGS+=("-DENABLE_COVERAGE=ON")
fi

echo "Configuring CMake"
cmake -S . -B "$BUILD_DIR" "${CMAKE_FLAGS[@]}"

echo "Building game and tests"
cmake --build "$BUILD_DIR"

echo "✅  Build completed successfully!"
