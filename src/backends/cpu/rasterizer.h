#ifndef LABFONT_CPU_RASTERIZER_H
#define LABFONT_CPU_RASTERIZER_H

#include "core/internal_types.h"
#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace labfont {
namespace cpu {

// Utility functions
inline float Clamp(float value, float min, float max) {
    return value < min ? min : (value > max ? max : value);
}

// Vertex type for CPU rasterization
struct Vertex {
    float position[2];  // x, y
    float texcoord[2];  // u, v
    float color[4];     // r, g, b, a
};

// Convert lab_vertex_2TC to internal Vertex
inline Vertex ConvertVertex(const lab_vertex_2TC& v) {
    Vertex result;
    std::memcpy(result.position, v.position, sizeof(float) * 2);
    std::memcpy(result.texcoord, v.texcoord, sizeof(float) * 2);
    std::memcpy(result.color, v.color, sizeof(float) * 4);
    return result;
}


// Helper function to blend colors
inline void BlendPixel(uint8_t* pixel, const float* color, BlendMode blendMode) {
    // RGBA format (assuming 8 bits per channel)
    uint8_t r = static_cast<uint8_t>(color[0] * 255.0f);
    uint8_t g = static_cast<uint8_t>(color[1] * 255.0f);
    uint8_t b = static_cast<uint8_t>(color[2] * 255.0f);
    uint8_t a = static_cast<uint8_t>(color[3] * 255.0f);
    
    // Simple alpha blending for now
    if (blendMode == BlendMode::Alpha) {
        float alpha = color[3];
        float invAlpha = 1.0f - alpha;
        
        pixel[0] = static_cast<uint8_t>(pixel[0] * invAlpha + r * alpha);
        pixel[1] = static_cast<uint8_t>(pixel[1] * invAlpha + g * alpha);
        pixel[2] = static_cast<uint8_t>(pixel[2] * invAlpha + b * alpha);
        pixel[3] = std::max(pixel[3], a);
    } else {
        // Replace mode
        pixel[0] = r;
        pixel[1] = g;
        pixel[2] = b;
        pixel[3] = a;
    }
}

// Rasterization functions
void DrawTriangle(
    uint8_t* colorBuffer,
    uint8_t* depthBuffer,
    uint32_t width,
    uint32_t height,
    const lab_vertex_2TC* vertices,
    BlendMode blendMode
) {
    // Convert vertices
    Vertex cpuVertices[3];
    for (int i = 0; i < 3; ++i) {
        cpuVertices[i] = ConvertVertex(vertices[i]);
    }
    
    // Scale vertices to screen space
    float x0 = cpuVertices[0].position[0] * width;
    float y0 = cpuVertices[0].position[1] * height;
    float x1 = cpuVertices[1].position[0] * width;
    float y1 = cpuVertices[1].position[1] * height;
    float x2 = cpuVertices[2].position[0] * width;
    float y2 = cpuVertices[2].position[1] * height;
    
    // Compute bounding box
    int minX = static_cast<int>(std::floor(std::min({x0, x1, x2})));
    int minY = static_cast<int>(std::floor(std::min({y0, y1, y2})));
    int maxX = static_cast<int>(std::ceil(std::max({x0, x1, x2})));
    int maxY = static_cast<int>(std::ceil(std::max({y0, y1, y2})));
    
    // Clip to screen
    minX = std::max(0, minX);
    minY = std::max(0, minY);
    maxX = std::min(static_cast<int>(width) - 1, maxX);
    maxY = std::min(static_cast<int>(height) - 1, maxY);
    
    // Precompute edge functions
    float edge01 = (y0 - y1) * x2 + (x1 - x0) * y2 + (x0 * y1 - y0 * x1);
    float edge12 = (y1 - y2) * x0 + (x2 - x1) * y0 + (x1 * y2 - y1 * x2);
    float edge20 = (y2 - y0) * x1 + (x0 - x2) * y1 + (x2 * y0 - y2 * x0);
    
    // Check if triangle is backfacing
    if (edge01 + edge12 + edge20 < 0) {
        // Flip edges for backfacing triangles
        edge01 = -edge01;
        edge12 = -edge12;
        edge20 = -edge20;
    }
    
    // Rasterize
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            // Compute barycentric coordinates
            float w0 = ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2)) / edge01;
            float w1 = ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2)) / edge12;
            float w2 = 1.0f - w0 - w1;
            
            // Check if pixel is inside triangle
            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                // Interpolate color
                float color[4] = {0, 0, 0, 0};
                for (int i = 0; i < 4; ++i) {
                    color[i] = w0 * cpuVertices[0].color[i] + 
                               w1 * cpuVertices[1].color[i] + 
                               w2 * cpuVertices[2].color[i];
                }
                
                // Write pixel
                uint8_t* pixel = &colorBuffer[(y * width + x) * 4];
                BlendPixel(pixel, color, blendMode);
            }
        }
    }
}

void DrawLine(
    uint8_t* colorBuffer,
    uint32_t width,
    uint32_t height,
    const lab_vertex_2TC* vertices,
    float lineWidth,
    BlendMode blendMode
) {
    // Convert vertices
    Vertex cpuVertices[2];
    for (int i = 0; i < 2; ++i) {
        cpuVertices[i] = ConvertVertex(vertices[i]);
    }
    
    // Scale vertices to screen space
    float x0 = cpuVertices[0].position[0] * width;
    float y0 = cpuVertices[0].position[1] * height;
    float x1 = cpuVertices[1].position[0] * width;
    float y1 = cpuVertices[1].position[1] * height;
    
    // Bresenham's line algorithm
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
        std::swap(cpuVertices[0], cpuVertices[1]);
    }
    
    float dx = x1 - x0;
    float dy = std::abs(y1 - y0);
    float error = dx / 2.0f;
    
    int y = static_cast<int>(y0);
    int ystep = (y0 < y1) ? 1 : -1;
    
    // Line thickness
    int thickness = static_cast<int>(std::max(1.0f, lineWidth));
    int halfThickness = thickness / 2;
    
    for (int x = static_cast<int>(x0); x <= static_cast<int>(x1); ++x) {
        // Draw thick line by drawing multiple pixels perpendicular to the line
        for (int t = -halfThickness; t <= halfThickness; ++t) {
            int px, py;
            if (steep) {
                px = y + t;
                py = x;
            } else {
                px = x;
                py = y + t;
            }
            
            // Check bounds
            if (px >= 0 && px < static_cast<int>(width) && 
                py >= 0 && py < static_cast<int>(height)) {
                
                // Interpolate color based on position along the line
                float alpha = (x - x0) / dx;
                float color[4];
                for (int i = 0; i < 4; ++i) {
                    color[i] = (1.0f - alpha) * cpuVertices[0].color[i] + 
                               alpha * cpuVertices[1].color[i];
                }
                
                // Write pixel
                uint8_t* pixel = &colorBuffer[(py * width + px) * 4];
                BlendPixel(pixel, color, blendMode);
            }
        }
        
        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}

} // namespace cpu
} // namespace labfont

#endif // LABFONT_CPU_RASTERIZER_H
