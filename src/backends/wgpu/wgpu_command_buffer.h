#ifndef LABFONT_WGPU_COMMAND_BUFFER_H
#define LABFONT_WGPU_COMMAND_BUFFER_H

#include "wgpu_types.h"
#include "core/backend_types.h"
#include <vector>

namespace labfont {
namespace wgpu {

class WGPUDevice;
class WGPURenderTarget;

// Vertex data for WebGPU pipeline
struct WGPUVertex {
    float position[2];
    float texcoord[2];
    float color[4];
};

// Command buffer for recording and submitting WebGPU commands
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
    WGPUCommandEncoderRef m_commandEncoder;
    WGPURenderPassEncoderRef m_renderPassEncoder;
    WGPUBufferRef m_vertexBuffer;
    size_t m_vertexBufferCapacity;
    std::vector<WGPUVertex> m_vertexData;
    BlendMode m_currentBlendMode;
    bool m_inRenderPass;
    
    // Pipeline states for different blend modes
    WGPURenderPipelineRef m_trianglePipeline;
    WGPURenderPipelineRef m_linePipeline;
    WGPURenderPipelineRef m_alphaPipeline;
    WGPURenderPipelineRef m_additivePipeline;
    WGPURenderPipelineRef m_multiplyPipeline;
    WGPURenderPipelineRef m_screenPipeline;
    
    // Bind group layout and bind group for uniforms
    WGPUBindGroupLayoutRef m_bindGroupLayout;
    WGPUBindGroupRef m_bindGroup;
    
    // Helper methods
    WGPURenderPipelineRef GetPipelineForBlendMode(BlendMode mode);
    bool CreatePipelines();
    void DestroyPipelines();
};

} // namespace wgpu
} // namespace labfont

#endif // LABFONT_WGPU_COMMAND_BUFFER_H
