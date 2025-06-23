#ifndef LABFONT_METAL_COMMAND_BUFFER_H
#define LABFONT_METAL_COMMAND_BUFFER_H

#include "metal_types.h"
#include "core/backend_types.h"
#include "core/vertex.h"
#include "core/internal_types.h"
#include <vector>

namespace labfont {
namespace metal {

class MetalDevice;
class MetalRenderTarget;

// Vertex data for Metal pipeline
struct MetalVertex {
    float position[2];
    float texcoord[2];
    float color[4];

    MetalVertex() = default;

    MetalVertex(float px, float py, float u, float v, const float col[4]) {
        position[0] = px;
        position[1] = py;
        texcoord[0] = u;
        texcoord[1] = v;
        std::memcpy(color, col, sizeof(float) * 4);
    }
    
    // Conversion constructor from Vertex to MetalVertex
    MetalVertex(const Vertex& v) {
        position[0] = v.position[0];
        position[1] = v.position[1];
        texcoord[0] = v.texcoord[0];
        texcoord[1] = v.texcoord[1];
        color[0] = v.color[0];
        color[1] = v.color[1];
        color[2] = v.color[2];
        color[3] = v.color[3];
    }
};

// Command buffer for recording and submitting Metal commands
class MetalCommandBuffer {
public:
    MetalCommandBuffer(MetalDevice* device);
    ~MetalCommandBuffer();
    
    bool Begin();
    bool End();
    
    lab_result BeginRenderPass(MetalRenderTarget* target);
    void EndRenderPass();
    
    void SetBlendMode(BlendMode mode);
    void SetScissorRect(int32_t x, int32_t y, uint32_t width, uint32_t height);
    void SetViewport(float x, float y, float width, float height);
    
    void Clear(const float color[4]);
    void DrawTriangles(const Vertex* vertices, uint32_t vertexCount);
    void DrawLines(const Vertex* vertices, uint32_t vertexCount, float lineWidth);
    void BindTexture(lab_texture);
    
private:
    enum class DrawMode { None, Triangles, Lines };
    bool CreateVertexBuffer();
    void UpdateVertexBuffer();
    void Flush(DrawMode mode);

    DrawMode m_currentDrawMode;

    MetalDevice* m_device;
    MetalCommandBufferRef m_commandBuffer;
    MetalRenderCommandEncoderRef m_renderEncoder;
    MetalBufferRef m_vertexBuffer;
    size_t m_vertexBufferCapacity;
    std::vector<MetalVertex> m_vertexData;
    BlendMode m_currentBlendMode;
    lab_texture m_currentTexture;
    bool m_inRenderPass;
};

} // namespace metal
} // namespace labfont

#endif // LABFONT_METAL_COMMAND_BUFFER_H
