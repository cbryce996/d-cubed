#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Configuration
SOURCE_DIR="./assets/shaders"
OUTPUT_DIR="./assets/shaders/bin"
SDK="macosx"

# Ensure output directory exists
mkdir -p "$OUTPUT_DIR"

# Function to compile a shader stage
compile() {
    local NAME=$1
    local STAGE=$2 # vert or frag

    echo "=== Compiling: $NAME.$STAGE ==="

    # 1. GLSL to SPIR-V
    # Validates your code and creates a binary intermediate
    glslangValidator -V "$SOURCE_DIR/$NAME.$STAGE" -o "$OUTPUT_DIR/$NAME.$STAGE.spv"

    # 2. SPIR-V to Metal Source (.metal)
    # We do NOT use --reflect here so it outputs clean Metal code
    spirv-cross "$OUTPUT_DIR/$NAME.$STAGE.spv" \
        --msl \
        --output "$OUTPUT_DIR/$NAME.$STAGE.metal"

    # 3. SPIR-V to Reflection Metadata (.json)
    # We use > to redirect the terminal output into a file
    spirv-cross "$OUTPUT_DIR/$NAME.$STAGE.spv" --reflect > "$OUTPUT_DIR/$NAME.$STAGE.json"

    # 4. Metal Source to AIR (Apple Intermediate Representation)
    # This now receives a valid .metal file, not JSON
    xcrun -sdk $SDK metal -c \
        "$OUTPUT_DIR/$NAME.$STAGE.metal" \
        -o "$OUTPUT_DIR/$NAME.$STAGE.air"

    # 5. AIR to Metallib (The binary SDL3_GPU actually loads)
    xcrun -sdk $SDK metallib \
        "$OUTPUT_DIR/$NAME.$STAGE.air" \
        -o "$OUTPUT_DIR/$NAME.$STAGE.metallib"

    # Clean up temporary build artifacts
    # Keep the .json and .metallib; delete the rest
    rm "$OUTPUT_DIR/$NAME.$STAGE.spv"
    rm "$OUTPUT_DIR/$NAME.$STAGE.metal"
    rm "$OUTPUT_DIR/$NAME.$STAGE.air"

    echo "Success: Created $NAME.$STAGE.metallib and $NAME.$STAGE.json"
}

# --- Compile Shaders ---
compile "anomaly" "vert"
compile "anomaly" "frag"

echo "---------------------------------------"
echo "All shaders compiled successfully."