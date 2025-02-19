#ifndef LABFONT_WGPU_COMMAND_BUFFER_H
#define LABFONT_WGPU_COMMAND_BUFFER_H

#include "core/backend.h"
#include "wgpu_types.h"
#include "wgpu_vertex.h"
#include <webgpu/webgpu_cpp.h>
#include <vector>

namespace labfont {

class WebGPUDevice;
class WebGPURenderTarget;

class WGPUCommandBuffer {
public:
    WGPUCommandBuffer(const WebGPUDevice* device);
    ~WGPUCommandBuffer();
    
    bool Begin();
    bool End();
    
    bool BeginRenderPass(WebGPURenderTarget* target);
    void EndRenderPass();
    
    void Clear(const float color[4]);
    void DrawTriangles(const lab_vertex_2TC* vertices, uint32_t vertexCount);
    void DrawLines(const lab_vertex_2TC* vertices, uint32_t vertexCount, float lineWidth);
    
private:
    void UpdateVertexBuffer(const lab_vertex_2TC* vertices, uint32_t vertexCount);
    
    const WebGPUDevice* m_device;
    WGPUCommandEncoder m_commandEncoder;
    WGPURenderPassEncoder m_renderPassEncoder;
    WGPUBuffer m_vertexBuffer;
    std::vector<WGPUVertex> m_vertexData;
    size_t m_vertexBufferSize;
};

} // namespace labfont

#endif // LABFONT_WGPU_COMMAND_BUFFER_H
