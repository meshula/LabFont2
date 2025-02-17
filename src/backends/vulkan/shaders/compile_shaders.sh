#!/bin/bash
set -e

# Check if VULKAN_SDK is set
if [ -z "$VULKAN_SDK" ]; then
    echo "Error: VULKAN_SDK environment variable not set"
    echo "Please install the Vulkan SDK and set VULKAN_SDK to its installation path"
    exit 1
fi

# Create compiled directory if it doesn't exist
mkdir -p compiled

# Compile vertex shader
$VULKAN_SDK/bin/glslc primitives.vert -o compiled/primitives.vert.spv

# Compile fragment shaders
$VULKAN_SDK/bin/glslc primitives.frag -o compiled/primitives.frag.spv
$VULKAN_SDK/bin/glslc lines.frag -o compiled/lines.frag.spv

echo "Shaders compiled successfully"
