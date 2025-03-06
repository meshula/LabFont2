#include "wgpu_command_buffer.h"

namespace labfont {

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
// Real implementation for Emscripten builds
WebGPUCommandBuffer::WebGPUCommandBuffer(const WebGPUDevice* device)
    : m_device(device)
    , m_encoder(nullptr)
    , m_renderPassEncoder(nullptr)
    , m_inRenderPass(false)
{
}

WebGPUCommandBuffer::~WebGPUCommandBuffer() {
    if (m_renderPassEncoder) {
        wgpuRenderPassEncoderRelease(m_renderPassEncoder);
    }
    if (m_encoder) {
        wgpuCommandEncoderRelease(m_encoder);
    }
}

bool WebGPUCommandBuffer::Begin() {
    m_encoder = wgpuDeviceCreateCommandEncoder(m_device->GetDevice(), nullptr);
    return m_encoder != nullptr;
}

bool WebGPUCommandBuffer::BeginRenderPass(WebGPURenderTarget* target) {
    if (!m_encoder || m_inRenderPass) {
        return false;
    }
    
    m_renderPassEncoder = wgpuCommandEncoderBeginRenderPass(m_encoder, target->GetRenderPassDescriptor());
    m_inRenderPass = m_renderPassEncoder != nullptr;
    return m_inRenderPass;
}

bool WebGPUCommandBuffer::EndRenderPass() {
    if (!m_inRenderPass || !m_renderPassEncoder) {
        return false;
    }
    
    wgpuRenderPassEncoderEnd(m_renderPassEncoder);
    wgpuRenderPassEncoderRelease(m_renderPassEncoder);
    m_renderPassEncoder = nullptr;
    m_inRenderPass = false;
    return true;
}

bool WebGPUCommandBuffer::End() {
    if (!m_encoder) {
        return false;
    }
    
    if (m_inRenderPass) {
        EndRenderPass();
    }
    
    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(m_encoder, nullptr);
    if (!commandBuffer) {
        return false;
    }
    
    wgpuQueueSubmit(m_device->GetQueue(), 1, &commandBuffer);
    wgpuCommandBufferRelease(commandBuffer);
    
    wgpuCommandEncoderRelease(m_encoder);
    m_encoder = nullptr;
    return true;
}

void WebGPUCommandBuffer::Clear(const float color[4]) {
    // In WebGPU, clearing is handled by the render pass descriptor
    // This function is a no-op as the clear color is set when beginning the render pass
}

void WebGPUCommandBuffer::DrawTriangles(const Vertex* vertices, uint32_t vertexCount) {
    if (!m_inRenderPass || !m_renderPassEncoder) {
        return;
    }
    
    // In a real implementation, we would:
    // 1. Create a buffer with the vertex data
    // 2. Set the pipeline state
    // 3. Set the vertex buffer
    // 4. Draw the triangles
    
    // For now, this is a stub implementation
}

void WebGPUCommandBuffer::DrawLines(const Vertex* vertices, uint32_t vertexCount, float lineWidth) {
    if (!m_inRenderPass || !m_renderPassEncoder) {
        return;
    }
    
    // In a real implementation, we would:
    // 1. Create a buffer with the vertex data
    // 2. Set the pipeline state
    // 3. Set the vertex buffer
    // 4. Draw the lines
    
    // For now, this is a stub implementation
}

void WebGPUCommandBuffer::SetBlendMode(BlendMode mode) {
    // In WebGPU, blend mode is set when creating the pipeline
    // This function would need to switch between different pipelines
    
    // For now, this is a stub implementation
}

void WebGPUCommandBuffer::SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (!m_inRenderPass || !m_renderPassEncoder) {
        return;
    }
    
    wgpuRenderPassEncoderSetScissorRect(m_renderPassEncoder, x, y, width, height);
}

void WebGPUCommandBuffer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (!m_inRenderPass || !m_renderPassEncoder) {
        return;
    }
    
    wgpuRenderPassEncoderSetViewport(m_renderPassEncoder, 
                                    static_cast<float>(x), 
                                    static_cast<float>(y), 
                                    static_cast<float>(width), 
                                    static_cast<float>(height), 
                                    0.0f, 1.0f);
}
#else
// Stub implementation for non-Emscripten builds
WebGPUCommandBuffer::WebGPUCommandBuffer(const WebGPUDevice* device)
    : m_device(device)
    , m_inRenderPass(false)
{
}

WebGPUCommandBuffer::~WebGPUCommandBuffer() {
}

bool WebGPUCommandBuffer::Begin() {
    return false;
}

bool WebGPUCommandBuffer::BeginRenderPass(WebGPURenderTarget* target) {
    return false;
}

bool WebGPUCommandBuffer::EndRenderPass() {
    return false;
}

bool WebGPUCommandBuffer::End() {
    return false;
}

void WebGPUCommandBuffer::Clear(const float color[4]) {
}

void WebGPUCommandBuffer::DrawTriangles(const Vertex* vertices, uint32_t vertexCount) {
}

void WebGPUCommandBuffer::DrawLines(const Vertex* vertices, uint32_t vertexCount, float lineWidth) {
}

void WebGPUCommandBuffer::SetBlendMode(BlendMode mode) {
}

void WebGPUCommandBuffer::SetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
}

void WebGPUCommandBuffer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
}
#endif

} // namespace labfont
