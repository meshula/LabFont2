#ifndef TEST_PATTERNS_H
#define TEST_PATTERNS_H

#include "labfont/labfont_types.h"
#include "labfont/labfont.h"

#ifdef __cplusplus
extern "C" {
#endif

// Draw a simple red triangle to test basic rendering
inline lab_operation_result test_draw_triangle(lab_context ctx) {
    // Create a render target
    lab_render_target target = nullptr;
    lab_render_target_desc target_desc = {
        .width = 512,
        .height = 512,
        .format = LAB_FORMAT_RGBA8_UNORM,
        .hasDepth = false
    };
    
    lab_operation_result result = lab_create_render_target(ctx, &target_desc, &target);
    if (result.error != LAB_ERROR_NONE) {
        return result;
    }
    
    // Begin frame
    result = lab_begin_frame(ctx);
    if (result.error != LAB_ERROR_NONE) {
        return result;
    }
    
    // Set render target
    result = lab_set_render_target(ctx, target);
    if (result.error != LAB_ERROR_NONE) {
        return result;
    }
    
    // Draw commands
    lab_vertex_2TC vertices[3] = {
        {{0.0f, 0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},  // Top
        {{-0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}, // Bottom left
        {{0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}   // Bottom right
    };
    
    lab_draw_command cmd = {
        .type = LAB_DRAW_COMMAND_TRIANGLES,
        .triangles = {
            .vertices = vertices,
            .vertexCount = 3
        }
    };
    
    result = lab_submit_commands(ctx, &cmd, 1);
    if (result.error != LAB_ERROR_NONE) {
        return result;
    }
    
    // End frame
    result = lab_end_frame(ctx);
    if (result.error != LAB_ERROR_NONE) {
        return result;
    }
    
    return (lab_operation_result){LAB_ERROR_NONE, nullptr};
}

#ifdef __cplusplus
}
#endif

#endif // TEST_PATTERNS_H
