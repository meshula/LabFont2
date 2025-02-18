# Test Patterns Plan

## Overview
This document outlines the plan for implementing test patterns in LabFont. Test patterns are standardized drawing operations used to verify the correctness of rendering across different backends.

## Building and Testing
```bash
# Build CPU backend tests
./build_mtl.sh  # Also builds CPU backend

# Build Metal backend tests
./build_mtl.sh

# Build WebGPU tests (after CPU and Metal are complete)
./build_wasm.sh
```

## Inception
The test patterns system will provide a consistent way to verify rendering across all backends. Starting with simple shapes like triangles, we'll build up to more complex patterns that exercise different aspects of the rendering system.

## Design

### Core Types
```c
// 2D vertex with texture coordinates and color
typedef struct {
    float position[2];  // x, y
    float texcoord[2];  // u, v
    float color[4];     // r, g, b, a
} lab_vertex_2TC;

// Render target description
typedef struct {
    uint32_t width;
    uint32_t height;
    lab_texture_format format;
    bool hasDepth;
} lab_render_target_desc;

// Render target handle
typedef struct lab_render_target_t* lab_render_target;

// Draw command types
typedef enum {
    LAB_DRAW_COMMAND_CLEAR,
    LAB_DRAW_COMMAND_TRIANGLES,
    LAB_DRAW_COMMAND_LINES
} lab_draw_command_type;

// Draw command data
typedef struct {
    lab_draw_command_type type;
    union {
        struct {
            float color[4];
        } clear;
        struct {
            const lab_vertex_2TC* vertices;
            uint32_t vertexCount;
        } triangles;
        struct {
            const lab_vertex_2TC* vertices;
            uint32_t vertexCount;
            float lineWidth;
        } lines;
    };
} lab_draw_command;
```

### Core Functions
```c
// Render target management
lab_result lab_create_render_target(lab_context ctx, const lab_render_target_desc* desc, lab_render_target* out_target);
lab_result lab_destroy_render_target(lab_context ctx, lab_render_target target);
lab_result lab_set_render_target(lab_context ctx, lab_render_target target);

// Frame management
lab_result lab_begin_frame(lab_context ctx);
lab_result lab_end_frame(lab_context ctx);

// Draw commands
lab_result lab_submit_commands(lab_context ctx, const lab_draw_command* commands, uint32_t commandCount);
```

## Implementation Plan

### Phase 1: CPU Backend
1. Add core types to labfont_types.h
2. Implement render target support in CPU backend
3. Implement draw command infrastructure in CPU backend
4. Create basic triangle test pattern
5. Verify CPU backend rendering

### Phase 2: Metal Backend
1. Port render target support to Metal
2. Port draw command infrastructure to Metal
3. Verify test patterns render identically to CPU

### Phase 3: WebGPU Backend
1. Port render target support to WebGPU
2. Port draw command infrastructure to WebGPU
3. Verify test patterns render identically across all backends

## TODO List

### CPU Backend
- [ ] Add lab_vertex_2TC to labfont_types.h
- [ ] Add render target types to labfont_types.h
- [ ] Add draw command types to labfont_types.h
- [ ] Implement CPU render target creation/management
- [ ] Implement CPU draw command processing
- [ ] Implement triangle rasterization
- [ ] Add test_draw_triangle to test_patterns.h
- [ ] Verify CPU rendering

### Metal Backend
- [ ] Port render target implementation to Metal
- [ ] Port draw command implementation to Metal
- [ ] Update Metal shaders for new vertex format
- [ ] Verify Metal rendering matches CPU

### WebGPU Backend
- [ ] Port render target implementation to WebGPU
- [ ] Port draw command implementation to WebGPU
- [ ] Update WebGPU shaders for new vertex format
- [ ] Verify WebGPU rendering matches CPU/Metal
