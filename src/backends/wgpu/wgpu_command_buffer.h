#ifndef LABFONT_WGPU_COMMAND_BUFFER_H
#define LABFONT_WGPU_COMMAND_BUFFER_H

#include "core/backend.h"
#include "core/vertex.h"
#include "wgpu_device.h"
#include "wgpu_render_target.h"

namespace labfont {

// WebGPU-specific command buffer implementation
class WebGPUCommandBuffer {
public:
    WebGPUCommandBuffer(const WebGPUDevice* device);
    ~WebGPUCommandBuffer();
    
    bool Begin();
    bool BeginRenderPass(WebGPURenderTarget* target);
    bool EndRenderPass();
    bool End();
    
    void Clear(const float color[4]);
    void DrawTriangles(const Vertex* vertices, uint32_t vertexCount);
    void DrawLines(const Vertex* vertices, uint32_t vertexCount, float lineWidth);
    void SetBlendMode(BlendMode mode);
    void SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    
private:
    const WebGPUDevice* m_device;
    
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    WGPUCommandEncoder m_encoder;
    WGPURenderPassEncoder m_renderPassEncoder;
#else
    void* m_encoder = nullptr;
    void* m_renderPassEncoder = nullptr;
#endif
    
    bool m_inRenderPass = false;
};

} // namespace labfont

#endif // LABFONT_WGPU_COMMAND_BUFFER_H
