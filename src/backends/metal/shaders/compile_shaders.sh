#!/bin/bash

# Exit on error
set -e

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Output directory for compiled shaders
OUTPUT_DIR="$SCRIPT_DIR/compiled"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Compile shaders
xcrun -sdk macosx metal -c "$SCRIPT_DIR/primitives.metal" -o "$OUTPUT_DIR/primitives.air"

# Create metal library
xcrun -sdk macosx metallib "$OUTPUT_DIR/primitives.air" -o "$OUTPUT_DIR/primitives.metallib"

# Clean up intermediate files
rm "$OUTPUT_DIR/primitives.air"

echo "Metal shaders compiled successfully"
