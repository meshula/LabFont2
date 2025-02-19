#include "wgpu_command_buffer.h"
#include "wgpu_backend.h"
#include <cstring>

namespace labfont {

WGPUCommandBuffer::WGPUCommandBuffer(const WebGPUDevice* device)
    : m_device(device)
    , m_commandEncoder(nullptr)
    , m_renderPassEncoder(nullptr)
    , m_vertexBuffer(nullptr)
    , m_vertexBufferSize(0)
{
}

WGPUCommandBuffer::~WGPUCommandBuffer() {
    if (m_vertexBuffer) {
        wgpuBufferDestroy(m_vertexBuffer);
    }
}

bool WGPUCommandBuffer::Begin() {
    WGPUCommandEncoderDescriptor encoderDesc = {};
    m_commandEncoder = wgpuDeviceCreateCommandEncoder(m_device->GetDevice(), &encoderDesc);
    return m_commandEncoder != nullptr;
}

bool WGPUCommandBuffer::End() {
    if (!m_commandEncoder) {
        return false;
    }
    
    WGPUCommandBufferDescriptor cmdBufferDesc = {};
    auto cmdBuffer = wgpuCommandEncoderFinish(m_commandEncoder, &cmdBufferDesc);
    auto queue = m_device->GetQueue();
    wgpuQueueSubmit(queue, 1, &cmdBuffer);
    wgpuCommandBufferRelease(cmdBuffer);
    wgpuCommandEncoderRelease(m_commandEncoder);
    m_commandEncoder = nullptr;
    return true;
}

bool WGPUCommandBuffer::BeginRenderPass(WebGPURenderTarget* target) {
    if (!m_commandEncoder || !target) {
        return false;
    }
    
    const WGPURenderPassDescriptor* renderPassDesc = target->GetRenderPassDesc();
    m_renderPassEncoder = wgpuCommandEncoderBeginRenderPass(m_commandEncoder, renderPassDesc);
    return m_renderPassEncoder != nullptr;
}

void WGPUCommandBuffer::EndRenderPass() {
    if (m_renderPassEncoder) {
        wgpuRenderPassEncoderEnd(m_renderPassEncoder);
        wgpuRenderPassEncoderRelease(m_renderPassEncoder);
        m_renderPassEncoder = nullptr;
    }
}

void WGPUCommandBuffer::Clear(const float color[4]) {
    if (!m_renderPassEncoder) {
        return;
    }
    
    // Clear color is set in render pass descriptor
}

void WGPUCommandBuffer::DrawTriangles(const lab_vertex_2TC* vertices, uint32_t vertexCount) {
    if (!m_renderPassEncoder || !vertices || vertexCount == 0) {
        return;
    }
    
    UpdateVertexBuffer(vertices, vertexCount);
    
    // Set vertex buffer and pipeline
    wgpuRenderPassEncoderSetVertexBuffer(m_renderPassEncoder, 0, m_vertexBuffer, 0, m_vertexBufferSize);
    
    // Draw
    wgpuRenderPassEncoderDraw(m_renderPassEncoder, vertexCount, 1, 0, 0);
}

void WGPUCommandBuffer::DrawLines(const lab_vertex_2TC* vertices, uint32_t vertexCount, float lineWidth) {
    if (!m_renderPassEncoder || !vertices || vertexCount == 0) {
        return;
    }
    
    UpdateVertexBuffer(vertices, vertexCount);
    
    // Set vertex buffer and pipeline
    wgpuRenderPassEncoderSetVertexBuffer(m_renderPassEncoder, 0, m_vertexBuffer, 0, m_vertexBufferSize);
    
    // Draw
    wgpuRenderPassEncoderDraw(m_renderPassEncoder, vertexCount, 1, 0, 0);
}

void WGPUCommandBuffer::UpdateVertexBuffer(const lab_vertex_2TC* vertices, uint32_t vertexCount) {
    // Convert vertices to WGPU format
    m_vertexData.clear();
    m_vertexData.reserve(vertexCount);
    for (uint32_t i = 0; i < vertexCount; ++i) {
        m_vertexData.push_back(WGPUVertex::FromLabVertex(vertices[i]));
    }
    
    size_t newSize = vertexCount * sizeof(WGPUVertex);
    
    // Create or resize buffer if needed
    if (!m_vertexBuffer || newSize > m_vertexBufferSize) {
        if (m_vertexBuffer) {
            wgpuBufferDestroy(m_vertexBuffer);
        }
        
        WGPUBufferDescriptor bufferDesc = {};
        bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        bufferDesc.size = newSize;
        m_vertexBuffer = wgpuDeviceCreateBuffer(m_device->GetDevice(), &bufferDesc);
        m_vertexBufferSize = newSize;
    }
    
    // Upload data
    wgpuQueueWriteBuffer(m_device->GetQueue(), m_vertexBuffer, 0, m_vertexData.data(), newSize);
}

} // namespace labfont
