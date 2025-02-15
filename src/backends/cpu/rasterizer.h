#ifndef LABFONT_CPU_RASTERIZER_H
#define LABFONT_CPU_RASTERIZER_H

#include <cstdint>
#include <algorithm>
#include <cmath>
#include "core/backend_types.h"

namespace labfont {
namespace cpu {

// Edge function for barycentric coordinates
inline float EdgeFunction(const float* a, const float* b, float x, float y) {
    return (x - a[0]) * (b[1] - a[1]) - (y - a[0]) * (b[0] - a[0]);
}

// Linear interpolation
template<typename T>
inline T Lerp(T a, T b, float t) {
    return a + (b - a) * t;
}

// Clamp value to range [min, max]
template<typename T>
inline T Clamp(T value, T min, T max) {
    return std::min(std::max(value, min), max);
}

// RGBA color blending
inline void BlendPixel(uint8_t* dst, const float* src, BlendMode mode) {
    float dst_r = dst[0] / 255.0f;
    float dst_g = dst[1] / 255.0f;
    float dst_b = dst[2] / 255.0f;
    float dst_a = dst[3] / 255.0f;
    
    float src_r = src[0];
    float src_g = src[1];
    float src_b = src[2];
    float src_a = src[3];
    
    float out_r, out_g, out_b, out_a;
    
    switch (mode) {
        case BlendMode::None:
            out_r = src_r;
            out_g = src_g;
            out_b = src_b;
            out_a = src_a;
            break;
            
        case BlendMode::Alpha:
            out_a = src_a + dst_a * (1.0f - src_a);
            if (out_a > 0.0f) {
                out_r = (src_r * src_a + dst_r * dst_a * (1.0f - src_a)) / out_a;
                out_g = (src_g * src_a + dst_g * dst_a * (1.0f - src_a)) / out_a;
                out_b = (src_b * src_a + dst_b * dst_a * (1.0f - src_a)) / out_a;
            } else {
                out_r = out_g = out_b = 0.0f;
            }
            break;
            
        case BlendMode::Additive:
            out_r = std::min(src_r + dst_r, 1.0f);
            out_g = std::min(src_g + dst_g, 1.0f);
            out_b = std::min(src_b + dst_b, 1.0f);
            out_a = std::min(src_a + dst_a, 1.0f);
            break;
            
        case BlendMode::Multiply:
            out_r = src_r * dst_r;
            out_g = src_g * dst_g;
            out_b = src_b * dst_b;
            out_a = src_a * dst_a;
            break;
            
        case BlendMode::Screen:
            out_r = 1.0f - (1.0f - src_r) * (1.0f - dst_r);
            out_g = 1.0f - (1.0f - src_g) * (1.0f - dst_g);
            out_b = 1.0f - (1.0f - src_b) * (1.0f - dst_b);
            out_a = 1.0f - (1.0f - src_a) * (1.0f - dst_a);
            break;
    }
    
    dst[0] = static_cast<uint8_t>(Clamp(out_r * 255.0f, 0.0f, 255.0f));
    dst[1] = static_cast<uint8_t>(Clamp(out_g * 255.0f, 0.0f, 255.0f));
    dst[2] = static_cast<uint8_t>(Clamp(out_b * 255.0f, 0.0f, 255.0f));
    dst[3] = static_cast<uint8_t>(Clamp(out_a * 255.0f, 0.0f, 255.0f));
}

// Draw a triangle with interpolated vertex attributes
inline void DrawTriangle(
    uint8_t* colorBuffer,
    float* depthBuffer,
    uint32_t width,
    uint32_t height,
    const Vertex* vertices,
    BlendMode blendMode)
{
    // Compute bounding box
    float minX = std::min(std::min(vertices[0].position[0], vertices[1].position[0]), vertices[2].position[0]);
    float minY = std::min(std::min(vertices[0].position[1], vertices[1].position[1]), vertices[2].position[1]);
    float maxX = std::max(std::max(vertices[0].position[0], vertices[1].position[0]), vertices[2].position[0]);
    float maxY = std::max(std::max(vertices[0].position[1], vertices[1].position[1]), vertices[2].position[1]);
    
    // Clip to viewport
    int32_t startX = std::max(static_cast<int32_t>(std::floor(minX)), 0);
    int32_t startY = std::max(static_cast<int32_t>(std::floor(minY)), 0);
    int32_t endX = std::min(static_cast<int32_t>(std::ceil(maxX)), static_cast<int32_t>(width));
    int32_t endY = std::min(static_cast<int32_t>(std::ceil(maxY)), static_cast<int32_t>(height));
    
    // Compute edge equations
    float area = EdgeFunction(vertices[0].position, vertices[1].position, vertices[2].position[0], vertices[2].position[1]);
    if (std::abs(area) < 1e-6f) return; // Degenerate triangle
    float invArea = 1.0f / area;
    
    // Rasterize
    for (int32_t y = startY; y < endY; ++y) {
        for (int32_t x = startX; x < endX; ++x) {
            float px = x + 0.5f;
            float py = y + 0.5f;
            
            // Compute barycentric coordinates
            float w0 = EdgeFunction(vertices[1].position, vertices[2].position, px, py) * invArea;
            float w1 = EdgeFunction(vertices[2].position, vertices[0].position, px, py) * invArea;
            float w2 = EdgeFunction(vertices[0].position, vertices[1].position, px, py) * invArea;
            
            // Check if pixel is inside triangle
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                // Interpolate color
                float color[4];
                for (int i = 0; i < 4; ++i) {
                    color[i] = w0 * vertices[0].color[i] +
                              w1 * vertices[1].color[i] +
                              w2 * vertices[2].color[i];
                }
                
                // Write pixel
                size_t offset = (y * width + x) * 4;
                BlendPixel(&colorBuffer[offset], color, blendMode);
            }
        }
    }
}

// Draw a line with the specified width
inline void DrawLine(
    uint8_t* colorBuffer,
    uint32_t width,
    uint32_t height,
    const Vertex* vertices,
    float lineWidth,
    BlendMode blendMode)
{
    // Convert line to triangle strip
    float dx = vertices[1].position[0] - vertices[0].position[0];
    float dy = vertices[1].position[1] - vertices[0].position[1];
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length < 1e-6f) return; // Degenerate line
    
    float nx = -dy / length * (lineWidth * 0.5f);
    float ny = dx / length * (lineWidth * 0.5f);
    
    Vertex strip[4] = {
        // Top left
        {{vertices[0].position[0] + nx, vertices[0].position[1] + ny},
         {0.0f, 0.0f},
         {vertices[0].color[0], vertices[0].color[1], vertices[0].color[2], vertices[0].color[3]}},
        // Bottom left
        {{vertices[0].position[0] - nx, vertices[0].position[1] - ny},
         {0.0f, 1.0f},
         {vertices[0].color[0], vertices[0].color[1], vertices[0].color[2], vertices[0].color[3]}},
        // Top right
        {{vertices[1].position[0] + nx, vertices[1].position[1] + ny},
         {1.0f, 0.0f},
         {vertices[1].color[0], vertices[1].color[1], vertices[1].color[2], vertices[1].color[3]}},
        // Bottom right
        {{vertices[1].position[0] - nx, vertices[1].position[1] - ny},
         {1.0f, 1.0f},
         {vertices[1].color[0], vertices[1].color[1], vertices[1].color[2], vertices[1].color[3]}}
    };
    
    // Draw as two triangles
    DrawTriangle(colorBuffer, nullptr, width, height, &strip[0], blendMode);
    DrawTriangle(colorBuffer, nullptr, width, height, &strip[1], blendMode);
}

} // namespace cpu
} // namespace labfont

#endif // LABFONT_CPU_RASTERIZER_H
