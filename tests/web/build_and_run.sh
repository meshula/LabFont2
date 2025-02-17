#!/bin/bash
set -e

# Check if emscripten is installed
if ! command -v emcc &> /dev/null; then
    echo "Emscripten not found. Please install it first:"
    echo "git clone https://github.com/emscripten-core/emsdk.git"
    echo "cd emsdk"
    echo "./emsdk install latest"
    echo "./emsdk activate latest"
    echo "source ./emsdk_env.sh"
    exit 1
fi

# Create build directory
BUILD_DIR="build_wasm"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure with CMake
emcmake cmake .. \
    -DLABFONT_ENABLE_WGPU=ON \
    -DCMAKE_BUILD_TYPE=Debug

# Build
emmake make

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
echo "Please open Chrome and navigate to http://localhost:8000/tests/labfont_wasm_tests.html"
echo "Press Ctrl+C to stop the server"
python3 serve.py
