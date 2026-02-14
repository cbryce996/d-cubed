#!/bin/bash

set -e

SOURCE_DIR="./assets/shaders"
OUTPUT_DIR="./assets/shaders/bin"
SDK="macosx"

mkdir -p "$OUTPUT_DIR"

compile() {
    local FULL_PATH=$1
    local FILENAME=$(basename -- "$FULL_PATH")
    local NAME="${FILENAME%.*}"
    local STAGE="${FILENAME##*.}"

    echo "=== Compiling: $FULL_PATH ==="

    # 1. GLSL to SPIR-V
    glslangValidator -V "$FULL_PATH" -o "$OUTPUT_DIR/$NAME.$STAGE.spv"

    # 2. SPIR-V to Metal Source (.metal)
    spirv-cross "$OUTPUT_DIR/$NAME.$STAGE.spv" \
        --msl \
        --msl-decoration-binding \
        --output "$OUTPUT_DIR/$NAME.$STAGE.metal"

    # 3. SPIR-V to Reflection Metadata (.json)
    spirv-cross "$OUTPUT_DIR/$NAME.$STAGE.spv" \
        --reflect > "$OUTPUT_DIR/$NAME.$STAGE.json"

    # 4. Metal Source to AIR
    xcrun -sdk $SDK metal -c \
        "$OUTPUT_DIR/$NAME.$STAGE.metal" \
        -o "$OUTPUT_DIR/$NAME.$STAGE.air"

    # 5. AIR to Metallib
    xcrun -sdk $SDK metallib \
        "$OUTPUT_DIR/$NAME.$STAGE.air" \
        -o "$OUTPUT_DIR/$NAME.$STAGE.metallib"

    # Clean up temporary build artifacts
    rm "$OUTPUT_DIR/$NAME.$STAGE.spv"
    rm "$OUTPUT_DIR/$NAME.$STAGE.metal"
    rm "$OUTPUT_DIR/$NAME.$STAGE.air"

    echo "Success: $NAME.$STAGE.metallib"
}

echo "Scanning $SOURCE_DIR recursively for shaders..."

find "$SOURCE_DIR" -type f \( -name "*.vert" -o -name "*.frag" \) | while read -r shader_file; do
    compile "$shader_file"
done

echo "---------------------------------------"
echo "All shaders compiled successfully."
