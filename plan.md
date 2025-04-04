# LabFont v2

## Collaborative Workflow
This section describes how we work together and use this plan.md file:

1. Plan.md serves multiple purposes:
   - Project documentation and planning
   - Task tracking and progress
   - Methodology guide
   - Crash recovery reference

2. For each work session:
   - Review the plan to understand current state
   - Identify next task or continue interrupted task
   - Follow test-driven development process
   - Update plan with progress and learnings

3. During development:
   - Implement functionality in small, testable increments
   - Create/update tests for new functionality
   - Run ALL tests after each significant change
   - Document test results and any issues found

4. After implementing features:
   - Run the complete test suite
   - Verify both new and existing functionality
   - Update plan.md with progress
   - Commit changes with descriptive messages

5. Crash Recovery Process:
   - Read this section to understand workflow
   - Review plan.md to identify current state
   - Check recent changes in relevant files
   - Run tests to verify system state
   - Continue from last known good state

6. Test Execution:
   - Run tests frequently during development
   - Use test output to verify progress
   - Add print statements to tests for clarity
   - Document test results in commits

Remember: The plan is a living document that evolves with the project, but we preserve the Inception section for historical context.

## Inception
We are going to write a new, modern version of LabFont, in the labfont_v2 directory. The existing API and source may be reviewed in the src directory, but we will not modify it. All work will be new, and in the labfont_v2 directory, including CMake scripts and so on. 
We are going to take inspiration for required functionality from the resources in the ref directory, although we will not specifically use that technology.
We are going to design an API that lets render styled text, as well as perform a number of immediate mode drawing operations, as inspired by "ref".
Our library should have a C interface, although the implementation may use C++. This is to enable easy language bindings.
We are going to want a Metal, WGPU, Vulkan, and DX11 backend, and the abstraction mustn't leak to the interface.

For each task in our project plan:
1. Implement the functionality
2. Create comprehensive unit tests
3. Build and run ALL tests to verify:
   - New functionality works correctly
   - No regression in existing functionality
4. When all tests pass:
   - Mark the task as complete with [X] in plan.md
   - Commit both the implementation and test results
5. Proceed to the next task

When modifying files:
1. Always verify file content integrity after modifications
2. Ensure no accidental truncation or content loss occurred
3. Review the complete file content in the tool result

We will also checkpoint the plan in git whenever we modify it. We will preserve the Inception section as is so that we understand how this project came to be.

We will initially run tests with munit. If we need to visual testing, we will discuss strategies later. Perhaps we can use an image understanding framework to check that a generated image contains, for example, a yellow circle on a black background, or text that reads "Hello world".

We think we will likely need fontstash, stb_truetype, and stb_image as external dependencies, any others we must pause and verify before use.

At any point during development we will rewrite this plan as necessary to facilitate the work or clarify elements, again, the Inception section we will keep in the interest of history.

Backend Testing Strategy:
We will implement headless testing for the backends, avoiding the need for windowed or interactive interfaces. This will allow for automated testing of all graphics functionality. Examples of tests include:

1. Texture Operations
   - Create textures of various formats (8-bit integer R/RGB/RGBA, f16, f32)
   - Write test patterns (e.g., checkerboard) to textures
   - Commit to GPU memory
   - Read back and verify contents

2. Drawing Operations
   - Create textures as render targets
   - Perform drawing operations (rectangles, lines, etc.)
   - Read back results and verify against expected patterns
   - Test different blend modes and states

3. Render Target Management
   - Create and switch between render targets
   - Verify correct state handling
   - Test multiple render target configurations

4. Memory Management
   - Verify proper allocation/deallocation of GPU resources
   - Test resource lifetime management
   - Monitor for memory leaks

This testing strategy will ensure consistent behavior across all backends while maintaining automated verification.

## Design Document

### Core Architecture
1. C Interface Layer
   - Pure C header for public API
   - Opaque handles for resources (fonts, contexts, etc.)
   - Error handling through return codes
   - Thread safety considerations

2. Backend Abstraction
   - Abstract rendering interface
   - Backend-specific implementations (Metal, WGPU, Vulkan, DX11)
   - Resource management per backend
   - Command buffer/queue management

3. Text Rendering System
   - Font loading and caching
   - Glyph atlas management
   - Text shaping and layout
   - Style management (color, size, weight, etc.)

4. Immediate Mode Drawing
   - Basic primitives (lines, rectangles, circles)
   - Path drawing
   - Fill and stroke operations
   - Transformation stack

### API Design
```c
// Context Management
lab_context* lab_create_context(lab_backend_type type, lab_context_desc* desc);
void lab_destroy_context(lab_context* ctx);

// Font Management
lab_font* lab_load_font(lab_context* ctx, const char* path);
void lab_destroy_font(lab_font* font);

// Text Rendering
void lab_begin_text(lab_context* ctx);
void lab_set_font(lab_context* ctx, lab_font* font);
void lab_set_font_size(lab_context* ctx, float size);
void lab_set_text_color(lab_context* ctx, lab_color color);
void lab_draw_text(lab_context* ctx, float x, float y, const char* text);
void lab_end_text(lab_context* ctx);

// Immediate Mode Drawing
void lab_begin_draw(lab_context* ctx);
void lab_set_color(lab_context* ctx, lab_color color);
void lab_set_line_width(lab_context* ctx, float width);
void lab_draw_line(lab_context* ctx, float x1, float y1, float x2, float y2);
void lab_draw_rect(lab_context* ctx, float x, float y, float w, float h);
void lab_draw_circle(lab_context* ctx, float x, float y, float radius);
void lab_end_draw(lab_context* ctx);
```

## Directory Layout and Build

```
labfont_v2/
├── include/
│   └── labfont/
│       ├── labfont.h          # Public C API
│       └── labfont_types.h    # Public types and constants
├── src/
│   ├── core/
│   │   ├── context.cpp        # Context management
│   │   ├── font.cpp          # Font loading and management
│   │   └── atlas.cpp         # Texture atlas management
│   ├── backends/
│   │   ├── metal/
│   │   ├── wgpu/
│   │   ├── vulkan/
│   │   └── dx11/
│   ├── text/
│   │   ├── shaping.cpp       # Text shaping
│   │   └── layout.cpp        # Text layout
│   └── draw/
│       ├── primitives.cpp    # Drawing primitives
│       └── path.cpp          # Path operations
├── tests/
│   ├── unit/                 # Unit tests
│   └── visual/              # Visual tests
├── examples/
│   ├── basic/              # Basic usage examples
│   └── advanced/           # Advanced features
└── CMakeLists.txt         # Main build script
```

## External Dependencies
- fontstash: Text rendering and font management
- stb_truetype: TrueType font loading
- stb_image: Image loading for textures
- munit: Unit testing framework

## Project Plan

[X] 1. Initial Setup
    [X] Create directory structure
    [X] Setup CMake build system
    [X] Add external dependencies
    [X] Create initial CI pipeline

[ ] 2. Core Implementation
    [X] Context management
    [X] Resource handling
    [X] Error handling system
    [X] Basic memory management

[ ] 3. Backend Implementation
    [X] Abstract backend interface
        - Texture management (create, update, read back)
        - Render target handling
        - Draw command interface
        - State management (blend modes, etc.)
        - Memory management interface
    [X] Backend Testing Infrastructure
        - Test pattern generation
        - Pixel buffer comparison utilities
        - GPU resource verification
    [X] CPU backend
        - Software rasterization
        - Memory-based textures and render targets
        - Reference implementation for testing
    [X] Metal backend
        - Core implementation
        - Headless testing support
        - Resource management
    [ ] WGPU backend (Browser-based)
        - Core implementation targeting browser's WebGPU API
        - Browser-based testing via Emscripten
        - Resource management
        - WebAssembly build configuration
    [ ] Vulkan backend
        - Core implementation
        - Headless testing support
        - Resource management
    [ ] DX11 backend
        - Core implementation
        - Headless testing support
        - Resource management

[ ] 4. Text Rendering
    [ ] Font loading
    [ ] Glyph atlas management
    [ ] Basic text rendering
    [ ] Text styling
    [ ] Text layout

[ ] 5. Immediate Mode Drawing
    [ ] Basic primitives
    [ ] Path operations
    [ ] Transformation system
    [ ] State management

[ ] 6. Testing
    [ ] Unit test framework setup
    [ ] Core functionality tests
    [ ] Backend-specific tests
    [ ] Text rendering tests
    [ ] Drawing operation tests
    [ ] Visual test framework
    [ ] Performance benchmarks

[ ] 7. Documentation
    [ ] API documentation
    [ ] Usage examples
    [ ] Backend-specific notes
    [ ] Build instructions

## Testing Process

1. Unit Testing
   - Using munit framework
   - Test categories:
     * Core functionality (context, resources, memory)
     * Backend operations (textures, render targets)
     * Text rendering and layout
     * Drawing operations
     * Error handling and recovery

2. Backend Testing
   - Headless testing framework
   - Test categories:
     * Texture formats and operations
     * Draw command verification
     * Render target management
     * State and blend modes
     * Resource lifetime and cleanup
   - Verification methods:
     * Pattern generation and validation
     * Pixel buffer comparison
     * Memory tracking
     * Error injection and recovery

3. Visual Testing
   - Automated image comparison
   - Test cases:
     * Basic shapes and primitives
     * Text rendering accuracy
     * Font styling
     * Complex layouts

3. Performance Testing
   - Benchmark suite for:
     * Text rendering performance
     * Drawing operation speed
     * Resource management efficiency
     * Backend comparison

4. Continuous Integration
   - Automated build testing
   - Unit test execution
   - Visual test verification
   - Performance benchmark tracking
