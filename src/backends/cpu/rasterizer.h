#ifndef LABFONT_CPU_RASTERIZER_H
#define LABFONT_CPU_RASTERIZER_H

#include "core/internal_types.h"
#include <vector>
#include <cstdint>

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

    // TODO: Implement triangle rasterization
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

    // TODO: Implement line rasterization
}

} // namespace cpu
} // namespace labfont

#endif // LABFONT_CPU_RASTERIZER_H
