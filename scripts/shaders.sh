#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Configuration
SOURCE_DIR="./shaders"
OUTPUT_DIR="./assets/shaders"
SDK="macosx" # Change to "iphoneos" if targeting mobile

# Ensure output directory exists
mkdir -p "$OUTPUT_DIR"

# Function to compile a shader stage
compile() {
    local NAME=$1
    local STAGE=$2 # vert or frag

    echo "Processing: $NAME.$STAGE"

    # 1. GLSL to SPIR-V (Validation & Intermediate format)
    # This checks for syntax errors before anything else
    glslangValidator -V "$SOURCE_DIR/$NAME.$STAGE" -o "$OUTPUT_DIR/$NAME.$STAGE.spv"

    # 2. SPIR-V to Metal (MSL) + Metadata Reflection
    # This generates the .json file your C++ code will read
    spirv-cross "$OUTPUT_DIR/$NAME.$STAGE.spv" \
        --msl \
        --output "$OUTPUT_DIR/$NAME.$STAGE.metal" \
        --reflect

    # 3. Metal to AIR (Apple Intermediate Representation)
    xcrun -sdk $SDK metal -c \
        "$OUTPUT_DIR/$NAME.$STAGE.metal" \
        -o "$OUTPUT_DIR/$NAME.$STAGE.air"

    # 4. AIR to Metallib (The final binary SDL3_GPU loads)
    xcrun -sdk $SDK metallib \
        "$OUTPUT_DIR/$NAME.$STAGE.air" \
        -o "$OUTPUT_DIR/$NAME.$STAGE.metallib"

    # Clean up temporary files (optional)
    rm "$OUTPUT_DIR/$NAME.$STAGE.air"
    rm "$OUTPUT_DIR/$NAME.$STAGE.metal"
    rm "$OUTPUT_DIR/$NAME.$STAGE.spv"

    echo "Done: $NAME.$STAGE"
}

# --- Compile your Nebula Shaders ---
compile "simple" "vert"
compile "simple" "frag"

echo "All shaders compiled successfully."