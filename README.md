# LabFont v2

A modern text rendering and immediate mode drawing library with multiple backend support.

## Features

- Pure C interface with C++ implementation
- Multiple backend support (Metal, WGPU, Vulkan, DX11)
- Text rendering with styling and layout
- Immediate mode drawing operations
- Thread-safe design

## Dependencies

The following dependencies are included in the `third_party` directory:
- [fontstash](https://github.com/memononen/fontstash) - Text rendering and font management
- [stb_truetype](https://github.com/nothings/stb) - TrueType font loading
- [stb_image](https://github.com/nothings/stb) - Image loading for textures

Test dependencies:
- [munit](https://github.com/nemequ/munit) - Unit testing framework (fetched automatically by CMake)

## Configuration

LabFont2 includes a configuration script that helps set up the build environment by detecting dependencies and generating appropriate build scripts.

```bash
# Run the configuration script
./configure.py
```

The script will:
1. Detect the Vulkan SDK, Emscripten, and GLFW installations
2. Generate platform-specific build scripts
3. Update the CMake configuration for better dependency handling
4. Provide guidance on missing dependencies

If a dependency is not found automatically, the script may prompt you to specify its location or provide instructions on how to install it.

## Building

After running the configuration script, you can use the generated build scripts:

```bash
# Build the core library (always works)
./build_core.sh  # or build_core.bat on Windows

# Build with Vulkan backend (if Vulkan SDK was found)
./build_vk.sh    # or build_vk.bat on Windows

# Build with Metal backend (macOS only)
./build_mtl.sh

# Build with WebAssembly and WebGPU (if Emscripten was found)
./build_wasm.sh  # or build_wasm.bat on Windows
```

Alternatively, you can use CMake directly:

```bash
mkdir build
cd build
cmake ..
make
```

### Build Options

- `LABFONT_BUILD_TESTS` - Build tests (ON by default)
- `LABFONT_BUILD_EXAMPLES` - Build examples (ON by default)
- `LABFONT_ENABLE_METAL` - Enable Metal backend
- `LABFONT_ENABLE_WGPU` - Enable WebGPU backend
- `LABFONT_ENABLE_VULKAN` - Enable Vulkan backend
- `LABFONT_ENABLE_DX11` - Enable DirectX 11 backend

Example:
```bash
cmake .. -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=ON
```

## Testing

```bash
cd build
ctest
```

### Test Organization

Unit tests follow the naming convention `test_BACKEND_feature.c/cpp`:

- `test_cpu_context.c` - Tests for context creation and management using the CPU backend
- `test_cpu_resources.c` - Tests for resource creation and management using the CPU backend
- `test_cpu_error.c` - Tests for error handling using the CPU backend
- `test_cpu_memory.c` - Tests for memory management using the CPU backend
- `test_cpu_backend.cpp` - Tests for CPU backend-specific functionality
- `test_metal_backend.cpp` - Tests for Metal backend-specific functionality
- `test_vulkan_backend.cpp` - Tests for Vulkan backend-specific functionality
- `test_wgpu_backend.cpp` - Tests for WebGPU backend-specific functionality

This organization ensures clear separation between tests for different backends and makes it easier to understand which backend is being tested.

## License

TBD


## Build Dependencies Notes

The following dependencies are required to build different components of LabFont2:

- **Vulkan SDK**: Found at `/Users/nick/VulkanSDK/1.4.304.1/macOS`
- **Emscripten**: Not found. Required for WebAssembly builds.
  - Follow the installation instructions at [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)
  - Run the configure.py script again after installation
- **GLFW**: Found

## Building

The configure.py script has generated build scripts for the detected dependencies:

- `build_core.sh`: Builds the core library without any backends
- `build_vk.sh`: Builds with the Vulkan backend
- `build_mtl.sh`: Builds with the Metal backend

### Example Build Scripts

- `build_examples_cpu.sh`: Builds examples with the CPU backend
- `build_examples_vulkan.sh`: Builds examples with the Vulkan backend
- `build_examples_metal.sh`: Builds examples with the Metal backend
