#!/bin/bash
set -e

# Setup Emscripten environment
source $HOME/emsdk/emsdk_env.sh

# Store original directory
ORIGINAL_DIR="$(pwd)"

# Create build directory
BUILD_DIR="build_wasm"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure with CMake
emcmake cmake .. -DLABFONT_ENABLE_WGPU=ON -DCMAKE_BUILD_TYPE=Debug

# Build
emmake make -j$(nproc 2>/dev/null || echo 4)  # Use 4 cores if nproc not available

echo "Build complete. Run 'npx http-server build_wasm' to start the server"
echo "Then open http://localhost:8080/labfont_wgpu_tests.html in Chrome"
