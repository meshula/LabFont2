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
- [cJSON](https://github.com/DaveGamble/cJSON) - JSON parsing for configuration

Test dependencies:
- [munit](https://github.com/nemequ/munit) - Unit testing framework (included in third_party)

## Configuration

LabFont2 includes a modular configuration system that helps set up the build environment by detecting dependencies and generating appropriate build scripts.

```bash
# Run the configuration script
python configure.py
```

The configuration script supports the following options:
- `--xcode` - Use Xcode generator instead of make (macOS only)
- `--build-dir` - Specify the directory to place the build scripts (default: "build")

The configuration process:
1. Detects required dependencies (Vulkan SDK, Emscripten, GLFW)
2. Generates appropriate build scripts for different backends and examples
3. Updates the README with dependency information
4. Updates CMake files to better handle dependencies

If a dependency is not found automatically, the script will prompt you to specify its location or provide instructions on how to install it.

## Build Scripts

After running the configuration script, you can use the generated build scripts:

### Core Library and Tests

```bash
# Build the core library and tests (always works)
./build/build_core.sh  # or build_core.bat on Windows
```

### Backend-Specific Builds with Tests

```bash
# Build with Vulkan backend and tests (if Vulkan SDK was found)
./build/build_vk.sh    # or build_vk.bat on Windows

# Build with Metal backend and tests (macOS only)
./build/build_mtl.sh

# Build with WebAssembly and WebGPU and tests (if Emscripten was found)
./build/build_wasm.sh  # or build_wasm.bat on Windows
```

### Example-Only Builds (No Tests)

```bash
# Build examples with CPU backend (no tests)
./build/build_examples_cpu.sh  # or build_examples_cpu.bat on Windows

# Build examples with Vulkan backend (no tests)
./build/build_examples_vulkan.sh  # or build_examples_vulkan.bat on Windows

# Build examples with Metal backend (no tests, macOS only)
./build/build_examples_metal.sh

# Build examples with WebGPU backend (no tests, if Emscripten was found)
./build/build_examples_wgpu.sh  # or build_examples_wgpu.bat on Windows
```

Alternatively, you can use CMake directly:

```bash
mkdir build
cd build
cmake ..
make
```

## Build Options

The following CMake options can be used to customize the build:

- `LABFONT_BUILD_TESTS` - Build tests (ON by default)
- `LABFONT_BUILD_EXAMPLES` - Build examples (ON by default)
- `LABFONT_ENABLE_METAL` - Enable Metal backend (macOS only)
- `LABFONT_ENABLE_WGPU` - Enable WebGPU backend
- `LABFONT_ENABLE_VULKAN` - Enable Vulkan backend
- `LABFONT_ENABLE_DX11` - Enable DirectX 11 backend
- `LAB_BACKEND` - Specify the backend to use for examples (CPU, VULKAN, METAL, WGPU)

Example:
```bash
cmake .. -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=ON -DLAB_BACKEND=METAL
```

## Testing

The test suite is organized by backend and feature, making it easier to understand which backend is being tested.

```bash
cd build
ctest
```

### Test Organization

Unit tests are organized into the following categories:

1. **Core Tests**: Tests for core functionality that is backend-independent
   - `test_cpu_context.c` - Tests for context creation and management
   - `test_cpu_resources.c` - Tests for resource creation and management
   - `test_cpu_error.c` - Tests for error handling
   - `test_cpu_memory.c` - Tests for memory management
   - `test_texture_loading.cpp` - Tests for texture loading functionality

2. **Backend-Specific Tests**: Tests for backend-specific functionality
   - `test_cpu_backend.cpp` - Tests for CPU backend
   - `test_metal_backend.cpp` - Tests for Metal backend
   - `test_vulkan_backend.cpp` - Tests for Vulkan backend
   - `test_wgpu_backend.cpp` - Tests for WebGPU backend
   - `test_dx11_backend.cpp` - Tests for DirectX 11 backend

3. **Feature-Specific Tests**: Tests for specific features
   - `test_labfont_style_parser.cpp` - Tests for the style parser

4. **Web Tests**: Tests for WebAssembly and WebGPU
   - `test_wgpu_browser.cpp` - Tests for WebGPU in the browser
   - `test_wgpu_main.cpp` - Main entry point for WebGPU tests

## Examples

LabFont2 includes several examples to demonstrate its capabilities:

### Basic Examples

1. **Drawing Example** (`examples/basic/drawing_main.cpp`):
   - Demonstrates how to create a window and draw a simple red triangle using LabFont's immediate mode drawing API
   - Shows basic setup of context, render target, and draw commands
   - Includes platform-specific code for displaying the rendered content

2. **Texture Example** (`examples/basic/texture_main.cpp`):
   - Shows how to load and display a texture using LabFont
   - Demonstrates texture loading, creating textured quads, and rendering them
   - Includes saving the render target to a file

### Rich Text Example

**Text Rendering** (`examples/rich_text/labfont_text_rendering.cpp`):
   - Demonstrates LabFont's rich text rendering capabilities
   - Shows how to use the text renderer with markup for styling
   - Includes examples of global styles, inline styles, and text layout options
   - Demonstrates features like bold, italic, colored text, and custom fonts

## License

BSD two clause
