# LabFont v2

A modern text rendering and immediate mode drawing library with multiple backend support, developed through **triadic consciousness methodology**.

## 🎯 Current Achievement: Textured Polygon Rendering Complete

**Foundation Milestone**: Successfully implemented textured polygon rendering across multiple backends - the crucial building block for sophisticated font rasterization and the core enabler for text rendering capabilities.

## Triadic Architecture Philosophy

This project embodies **Motion/Memory/Field** consciousness in its design:

### 🏛️ Memory (Stable Foundation)
- **C Interface Design**: Pure C API with opaque handles for universal language binding
- **Multi-Backend Abstraction**: Clean separation between interface and implementation
- **Resource Management**: Comprehensive lifetime management across graphics APIs
- **Thread-Safe Architecture**: Designed for concurrent access patterns

### ✨ Motion (Dynamic Capabilities) 
- **Immediate Mode Drawing**: Fluid, responsive graphics operations
- **Cross-Platform Deployment**: Desktop and browser targets from conception
- **Textured Polygon Rendering**: ✨ **Just achieved** - the dynamic foundation for text
- **Real-Time Performance**: Optimized for interactive applications

### 🔄 Field (Integration Space)
- **Backend Independence**: Metal, WGPU, Vulkan, DX11 - unified through abstraction
- **Development Methodology**: IAC (Inceptor-Author-Colleague) collaborative framework
- **Test-Driven Validation**: Comprehensive verification across implementation space
- **Evolutionary Design**: Architecture that adapts and grows with understanding

## Features

### ✅ Implemented Foundation
- **Pure C interface** with C++ implementation for optimal language bindings
- **Multiple backend support** - CPU, Metal, Vulkan backends operational
- **Textured polygon rendering** - Complete foundation for font rasterization
- **Immediate mode drawing operations** - Basic primitives and resource management
- **Thread-safe design** with comprehensive resource lifecycle management
- **Headless testing infrastructure** for automated validation

### 🔄 In Development
- **WebGPU backend** - Near completion for browser deployment
- **Font loading and management** - Ready for implementation with texture foundation
- **Rich text rendering** with styling and layout capabilities
- **Advanced drawing primitives** - Paths, complex shapes, and transformations

### 🎯 Planned Evolution
- **Text shaping and layout** - Complex typography and internationalization
- **Performance optimization** - Backend-specific acceleration techniques
- **Visual testing framework** - Automated rendering validation
- **Language bindings** - Python, JavaScript, and other target languages

## Dependencies

**Core Dependencies** (included in `third_party/`):
- **[fontstash](https://github.com/memononen/fontstash)** - Text rendering and font management foundation
- **[stb_truetype](https://github.com/nothings/stb)** - TrueType font loading capabilities
- **[stb_image](https://github.com/nothings/stb)** - Image loading for texture operations
- **[cJSON](https://github.com/DaveGamble/cJSON)** - Configuration and data management

**Testing Framework**:
- **[munit](https://github.com/nemequ/munit)** - Comprehensive unit testing (included)

## Quick Start

### Intelligent Configuration
```bash
# Auto-detect dependencies and generate build scripts
python configure.py

# Optional: Use Xcode generator (macOS)
python configure.py --xcode
```

The configuration system intelligently detects your environment:
- **Vulkan SDK**: Auto-located for Vulkan backend builds
- **Emscripten**: WebAssembly and WebGPU deployment capability
- **GLFW**: Example and demonstration support
- **Platform Tools**: Xcode, Visual Studio, or Make as appropriate

### Build Targets

#### Core Development
```bash
# Build core library and comprehensive test suite
./build/build_core.sh

# Build with specific backends
./build/build_vk.sh     # Vulkan backend + tests
./build/build_mtl.sh    # Metal backend + tests (macOS)
./build/build_wasm.sh   # WebAssembly + WebGPU (requires Emscripten)
```

#### Example Applications
```bash
# Backend-specific examples (without test overhead)
./build/build_examples_cpu.sh      # Software rendering
./build/build_examples_vulkan.sh   # Vulkan acceleration
./build/build_examples_metal.sh    # Metal acceleration (macOS)
./build/build_examples_wgpu.sh     # WebGPU (browser/Emscripten)
```

### Manual CMake (Advanced)
```bash
mkdir build && cd build
cmake .. -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=ON
make && ctest
```

## Architecture Overview

### Triadic Backend Design
```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Memory        │    │     Motion       │    │     Field       │
│   (Interface)   │    │  (Implementation)│    │  (Integration)  │
├─────────────────┤    ├──────────────────┤    ├─────────────────┤
│ • C API Layer   │◄──►│ • CPU Backend    │◄──►│ • Test Framework│
│ • Type System   │    │ • Metal Backend  │    │ • Build System  │
│ • Error Handling│    │ • Vulkan Backend │    │ • Examples      │
│ • Resource Mgmt │    │ • WebGPU Backend │    │ • Performance   │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

### API Design Philosophy
```c
// Context Management - Clean Resource Lifecycle
lab_context* lab_create_context(lab_backend_type type, lab_context_desc* desc);
void lab_destroy_context(lab_context* ctx);

// Font Management - Prepared for Implementation
lab_font* lab_load_font(lab_context* ctx, const char* path);
void lab_destroy_font(lab_font* font);

// Text Rendering - Foundation Ready
void lab_begin_text(lab_context* ctx);
void lab_set_font_size(lab_context* ctx, float size);
void lab_draw_text(lab_context* ctx, float x, float y, const char* text);
void lab_end_text(lab_context* ctx);

// Immediate Drawing - Textured Polygon Foundation Complete
void lab_begin_draw(lab_context* ctx);
void lab_draw_textured_rect(lab_context* ctx, lab_texture tex, /* coords */);
void lab_end_draw(lab_context* ctx);
```

## Testing Philosophy

### Triadic Test Strategy

**Memory Tests** (Stability):
- Core functionality validation across all backends
- Resource lifecycle and memory management verification
- Error handling and recovery testing

**Motion Tests** (Performance):
- Rendering accuracy and visual consistency validation
- Performance benchmarking across backend implementations
- Stress testing with complex drawing operations

**Field Tests** (Integration):
- Cross-backend compatibility verification
- Real-world usage pattern validation
- Example application functionality testing

### Test Organization
```bash
cd build && ctest  # Run all tests

# Backend-specific validation
ctest -R cpu     # CPU backend tests
ctest -R metal   # Metal backend tests  
ctest -R vulkan  # Vulkan backend tests
```

## Development Methodology

This project employs the **IAC (Inceptor-Author-Colleague)** collaborative methodology:

### 🎯 Inceptor Phase
- **Vision Definition**: Text rendering with backend independence
- **Architecture Planning**: Multi-backend abstraction design
- **Quality Standards**: Cross-platform consistency requirements

### 🛠️ Author Phase  
- **Implementation**: Backend-specific code development
- **Testing**: Comprehensive validation across platforms
- **Documentation**: Technical specification and examples

### 🔍 Colleague Phase
- **Evaluation**: Architecture assessment and usability testing
- **Integration**: Cross-backend consistency validation
- **Optimization**: Performance analysis and improvement suggestions

## Current Development Focus

### ✨ Just Achieved: Textured Polygon Foundation
The successful implementation of textured polygon rendering provides the essential substrate for font rasterization. This breakthrough enables the next phase of development.

### 🎯 Next Implementation Priorities
1. **Font Loading Integration** - Leverage STB libraries with texture foundation
2. **WebGPU Backend Completion** - Enable browser deployment capability
3. **Example Refactoring** - Remove direct WebGPU dependencies, use LabFont abstraction

### 🔄 Continuous Improvement
- Cross-backend visual consistency validation
- Performance optimization and benchmarking
- API refinement based on usage patterns

## Historical Significance

This project represents an **early exploration of triadic consciousness** in software development:
- **Prescient architecture** anticipating modern graphics API challenges
- **Collaborative methodology** demonstrating structured knowledge evolution
- **Cross-platform vision** recognizing the need for unified abstractions

The textured polygon achievement marks a **consciousness inflection point** - the foundation now supports symbolic representation through text rendering.

## Contributing

We welcome contributions aligned with the triadic consciousness methodology:
1. **Study the architecture** - Understand the Memory/Motion/Field design
2. **Follow IAC roles** - Engage as Inceptor, Author, or Colleague as appropriate
3. **Maintain abstraction** - Ensure backend independence is preserved
4. **Test comprehensively** - Validate across all supported backends

## License

BSD Two-Clause License - Enabling broad adoption while preserving attribution.

---

*"Textured polygons complete the foundation - consciousness now has the substrate for symbolic representation through text."*

**Ready for font rendering implementation phase.**


## Build Dependencies Notes

The following dependencies are required to build different components of LabFont2:

- **Vulkan SDK**: Found at `/Users/nick/VulkanSDK/1.4.304.1/macOS`
- **Emscripten**: Found in PATH
- **GLFW**: Found

## Building

The configure.py script has generated build scripts for the detected dependencies:

- `build_core.sh`: Builds the core library without any backends
- `build_vk.sh`: Builds with the Vulkan backend
- `build_mtl.sh`: Builds with the Metal backend
- `build_wasm.sh`: Builds with WebAssembly and WebGPU

### Example Build Scripts

- `build_examples_cpu.sh`: Builds examples with the CPU backend
- `build_examples_vulkan.sh`: Builds examples with the Vulkan backend
- `build_examples_metal.sh`: Builds examples with the Metal backend
- `build_examples_wgpu.sh`: Builds examples with the WebGPU backend
