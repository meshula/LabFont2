#ifndef LABFONT_METAL_COMMAND_BUFFER_H
#define LABFONT_METAL_COMMAND_BUFFER_H

#include "metal_types.h"
#include "core/backend_types.h"
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
};

// Command buffer for recording and submitting Metal commands
class MetalCommandBuffer {
public:
    MetalCommandBuffer(MetalDevice* device);
    ~MetalCommandBuffer();
    
    bool Begin();
    bool End();
    
    bool BeginRenderPass(MetalRenderTarget* target);
    void EndRenderPass();
    
    void SetBlendMode(BlendMode mode);
    void SetScissorRect(int32_t x, int32_t y, uint32_t width, uint32_t height);
    void SetViewport(float x, float y, float width, float height);
    
    void Clear(const float color[4]);
    void DrawTriangles(const Vertex* vertices, uint32_t vertexCount);
    void DrawLines(const Vertex* vertices, uint32_t vertexCount, float lineWidth);
    
private:
    bool CreateVertexBuffer();
    void UpdateVertexBuffer(const Vertex* vertices, uint32_t vertexCount);
    
    MetalDevice* m_device;
    MetalCommandBufferRef m_commandBuffer;
    MetalRenderCommandEncoderRef m_renderEncoder;
    MetalBufferRef m_vertexBuffer;
    size_t m_vertexBufferCapacity;
    std::vector<MetalVertex> m_vertexData;
    BlendMode m_currentBlendMode;
    bool m_inRenderPass;
};

} // namespace metal
} // namespace labfont

#endif // LABFONT_METAL_COMMAND_BUFFER_H
