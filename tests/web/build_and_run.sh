#!/bin/bash
set -e

# Store original directory
ORIGINAL_DIR="$(pwd)"

# Ensure EMSDK is installed
if [ ! -d "$HOME/emsdk" ]; then
    echo "Emscripten SDK not found. Installing..."
    git clone https://github.com/emscripten-core/emsdk.git "$HOME/emsdk"
    cd "$HOME/emsdk"
    ./emsdk install latest
    ./emsdk activate latest
    cd "$ORIGINAL_DIR"
fi

# Set up Emscripten environment dynamically
eval $(~/emsdk/emsdk construct_env)

# Verify `emcc` is available
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten is not properly set up."
    exit 1
fi

# Return to original directory in case we switched
cd "$ORIGINAL_DIR"

# Create build directory
BUILD_DIR="build_wasm"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure with CMake
emcmake cmake .. -DLABFONT_ENABLE_WGPU=ON -DCMAKE_BUILD_TYPE=Debug

# Build
emmake make -j$(nproc)  # Use all available cores

# Create a simple Python HTTP server script
cat > serve.py << 'EOF'
import http.server
import socketserver

class Handler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Add CORS headers
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        super().end_headers()

PORT = 8000
Handler.extensions_map['.wasm'] = 'application/wasm'

with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print(f"Serving at http://localhost:{PORT}")
    httpd.serve_forever()
EOF

# Start the server
echo "Starting local server..."
echo "Please open Chrome and navigate to http://localhost:8000/labfont_wasm_tests.html"
echo "Press Ctrl+C to stop the server"
python3 serve.py
