#ifndef TEST_PATTERNS_H
#define TEST_PATTERNS_H

#include "labfont/labfont_types.h"
#include "labfont/labfont.h"
#include <vector>
#include <cmath>
#include <cstring>

#ifdef __cplusplus
// C++ utility classes for test patterns
namespace PatternGenerator {
    template<typename T>
    std::vector<T> GenerateCheckerboard(
        uint32_t width, uint32_t height, 
        uint32_t blockSize,
        const T* color1, const T* color2, 
        uint32_t channels
    ) {
        std::vector<T> data(width * height * channels);
        
        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                bool isColor1 = ((x / blockSize) + (y / blockSize)) % 2 == 0;
                const T* color = isColor1 ? color1 : color2;
                
                for (uint32_t c = 0; c < channels; c++) {
                    data[(y * width + x) * channels + c] = color[c];
                }
            }
        }
        
        return data;
    }
}

namespace PixelComparator {
    template<typename T>
    bool CompareBuffers(
        const T* buffer1, const T* buffer2,
        uint32_t pixelCount, uint32_t channels,
        T tolerance
    ) {
        for (uint32_t i = 0; i < pixelCount * channels; i++) {
            T diff = std::abs(buffer1[i] - buffer2[i]);
            if (diff > tolerance) {
                return false;
            }
        }
        return true;
    }
}

extern "C" {
#endif

// Draw a simple red triangle to test basic rendering
inline lab_result test_draw_triangle(lab_context ctx) {
    // Create a render target
    lab_render_target target = nullptr;
    lab_render_target_desc target_desc = {
        .width = 512,
        .height = 512,
        .format = LAB_FORMAT_RGBA8_UNORM,
        .hasDepth = false
    };
    
    lab_result result = lab_create_render_target(ctx, &target_desc, &target);
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    // Begin frame
    result = lab_begin_frame(ctx);
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    // Set render target
    result = lab_set_render_target(ctx, target);
    if (result != LAB_RESULT_OK) {
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
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    // End frame
    result = lab_end_frame(ctx);
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    return LAB_RESULT_OK;
}

#ifdef __cplusplus
}
#endif

#endif // TEST_PATTERNS_H
