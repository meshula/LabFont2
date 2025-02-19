#ifndef LABFONT_WGPU_VERTEX_H
#define LABFONT_WGPU_VERTEX_H

#include "core/internal_types.h"
#include <webgpu/webgpu_cpp.h>

namespace labfont {

// Vertex type for WGPU backend
struct WGPUVertex {
    float position[2];  // x, y
    float texcoord[2];  // u, v
    float color[4];     // r, g, b, a

    // Convert from lab_vertex_2TC
    static WGPUVertex FromLabVertex(const lab_vertex_2TC& v) {
        WGPUVertex result;
        std::memcpy(result.position, v.position, sizeof(float) * 2);
        std::memcpy(result.texcoord, v.texcoord, sizeof(float) * 2);
        std::memcpy(result.color, v.color, sizeof(float) * 4);
        return result;
    }

    // Get vertex buffer layout
    static WGPUVertexBufferLayout GetLayout() {
        static const WGPUVertexAttribute attributes[] = {
            {
                .format = WGPUVertexFormat_Float32x2,
                .offset = offsetof(WGPUVertex, position),
                .shaderLocation = 0
            },
            {
                .format = WGPUVertexFormat_Float32x2,
                .offset = offsetof(WGPUVertex, texcoord),
                .shaderLocation = 1
            },
            {
                .format = WGPUVertexFormat_Float32x4,
                .offset = offsetof(WGPUVertex, color),
                .shaderLocation = 2
            }
        };

        WGPUVertexBufferLayout layout = {};
        layout.arrayStride = sizeof(WGPUVertex);
        layout.stepMode = WGPUVertexStepMode_Vertex;
        layout.attributeCount = 3;
        layout.attributes = attributes;
        return layout;
    }
};

} // namespace labfont

#endif // LABFONT_WGPU_VERTEX_H
