#ifndef LABFONT_WGPU_COMMAND_BUFFER_H
#define LABFONT_WGPU_COMMAND_BUFFER_H

#include "core/backend_types.h"
#include "wgpu_types.h"
#include <vector>
#include <emscripten.h>

namespace labfont {
namespace wgpu {

class WGPUDevice;

class WGPUCommandBuffer {
public:
    WGPUCommandBuffer(WGPUDevice* device);
    ~WGPUCommandBuffer();
    
    bool Begin();
    bool End();
    
    bool BeginRenderPass(WGPURenderTarget* target);
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
    
    WGPUDevice* m_device;
    WGPUCommandEncoder m_commandEncoder;
    WGPURenderPassEncoder m_renderPassEncoder;
    WGPUBuffer m_vertexBuffer;
    size_t m_vertexBufferCapacity;
    std::vector<WGPUVertex> m_vertexData;
    BlendMode m_currentBlendMode;
    bool m_inRenderPass;
};

} // namespace wgpu
} // namespace labfont

#endif // LABFONT_WGPU_COMMAND_BUFFER_H
