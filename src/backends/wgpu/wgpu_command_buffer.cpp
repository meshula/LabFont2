#include "wgpu_command_buffer.h"
#include "wgpu_backend.h"
#include <cstring>

namespace labfont {
namespace wgpu {

WGPUCommandBuffer::WGPUCommandBuffer(WGPUDevice* device)
    : m_device(device)
    , m_commandEncoder(nullptr)
    , m_renderPassEncoder(nullptr)
    , m_vertexBuffer(nullptr)
    , m_vertexBufferCapacity(0)
    , m_currentBlendMode(BlendMode::None)
    , m_inRenderPass(false)
    , m_trianglePipeline(nullptr)
    , m_linePipeline(nullptr)
    , m_alphaPipeline(nullptr)
    , m_additivePipeline(nullptr)
    , m_multiplyPipeline(nullptr)
    , m_screenPipeline(nullptr)
    , m_bindGroupLayout(nullptr)
    , m_bindGroup(nullptr)
{
    CreateVertexBuffer();
    CreatePipelines();
}

WGPUCommandBuffer::~WGPUCommandBuffer() {
    DestroyPipelines();
    
    if (m_vertexBuffer) {
        wgpuBufferDestroy(m_vertexBuffer);
    }
    
    if (m_bindGroup) {
        wgpuBindGroupRelease(m_bindGroup);
    }
    
    if (m_bindGroupLayout) {
        wgpuBindGroupLayoutRelease(m_bindGroupLayout);
    }
}

bool WGPUCommandBuffer::Begin() {
    WGPUCommandEncoderDescriptor encoderDesc = {};
    m_commandEncoder = wgpuDeviceCreateCommandEncoder(m_device->GetWGPUDevice(), &encoderDesc);
    return m_commandEncoder != nullptr;
}

bool WGPUCommandBuffer::End() {
    if (m_inRenderPass) {
        EndRenderPass();
    }
    
    if (m_commandEncoder) {
        WGPUCommandBufferDescriptor cmdBufferDesc = {};
        WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(m_commandEncoder, &cmdBufferDesc);
        wgpuQueueSubmit(m_device->GetQueue(), 1, &cmdBuffer);
        wgpuCommandBufferRelease(cmdBuffer);
        wgpuCommandEncoderRelease(m_commandEncoder);
        m_commandEncoder = nullptr;
    }
    
    return true;
}

bool WGPUCommandBuffer::BeginRenderPass(WGPURenderTarget* target) {
    if (m_inRenderPass) {
        EndRenderPass();
    }
    
    const auto& renderPassDesc = target->GetRenderPassDesc();
    m_renderPassEncoder = wgpuCommandEncoderBeginRenderPass(m_commandEncoder, &renderPassDesc);
    if (!m_renderPassEncoder) {
        return false;
    }
    
    m_inRenderPass = true;
    return true;
}

void WGPUCommandBuffer::EndRenderPass() {
    if (m_renderPassEncoder) {
        wgpuRenderPassEncoderEnd(m_renderPassEncoder);
        wgpuRenderPassEncoderRelease(m_renderPassEncoder);
        m_renderPassEncoder = nullptr;
    }
    m_inRenderPass = false;
}

void WGPUCommandBuffer::SetBlendMode(BlendMode mode) {
    m_currentBlendMode = mode;
    
    if (m_renderPassEncoder) {
        WGPURenderPipelineRef pipeline = GetPipelineForBlendMode(mode);
        if (pipeline) {
            wgpuRenderPassEncoderSetPipeline(m_renderPassEncoder, pipeline);
        }
    }
}

void WGPUCommandBuffer::SetScissorRect(int32_t x, int32_t y, uint32_t width, uint32_t height) {
    if (m_renderPassEncoder) {
        wgpuRenderPassEncoderSetScissorRect(m_renderPassEncoder, x, y, width, height);
    }
}

void WGPUCommandBuffer::SetViewport(float x, float y, float width, float height) {
    if (m_renderPassEncoder) {
        wgpuRenderPassEncoderSetViewport(m_renderPassEncoder, x, y, width, height, 0.0f, 1.0f);
    }
}

void WGPUCommandBuffer::Clear(const float color[4]) {
    // Clear is handled through render pass load action
}

void WGPUCommandBuffer::DrawTriangles(const Vertex* vertices, uint32_t vertexCount) {
    if (!m_renderPassEncoder || vertexCount == 0) {
        return;
    }
    
    // Convert vertices to WebGPU format
    m_vertexData.resize(vertexCount);
    for (uint32_t i = 0; i < vertexCount; ++i) {
        m_vertexData[i].position[0] = vertices[i].position[0];
        m_vertexData[i].position[1] = vertices[i].position[1];
        m_vertexData[i].texcoord[0] = vertices[i].texcoord[0];
        m_vertexData[i].texcoord[1] = vertices[i].texcoord[1];
        m_vertexData[i].color[0] = vertices[i].color[0];
        m_vertexData[i].color[1] = vertices[i].color[1];
        m_vertexData[i].color[2] = vertices[i].color[2];
        m_vertexData[i].color[3] = vertices[i].color[3];
    }
    
    // Update vertex buffer
    UpdateVertexBuffer(vertices, vertexCount);
    
    // Set vertex buffer and draw
    wgpuRenderPassEncoderSetVertexBuffer(m_renderPassEncoder, 0, m_vertexBuffer, 0, vertexCount * sizeof(WGPUVertex));
    wgpuRenderPassEncoderDraw(m_renderPassEncoder, vertexCount, 1, 0, 0);
}

void WGPUCommandBuffer::DrawLines(const Vertex* vertices, uint32_t vertexCount, float lineWidth) {
    if (!m_renderPassEncoder || vertexCount == 0) {
        return;
    }
    
    // Convert lines to triangles
    m_vertexData.clear();
    m_vertexData.reserve(vertexCount * 2);
    
    for (uint32_t i = 0; i < vertexCount; i += 2) {
        const Vertex& v0 = vertices[i];
        const Vertex& v1 = vertices[i + 1];
        
        // Calculate line direction and normal
        float dx = v1.position[0] - v0.position[0];
        float dy = v1.position[1] - v0.position[1];
        float length = std::sqrt(dx * dx + dy * dy);
        
        if (length < 1e-6f) continue;
        
        float nx = -dy / length * (lineWidth * 0.5f);
        float ny = dx / length * (lineWidth * 0.5f);
        
        // Add vertices for line segment quad
        WGPUVertex mv;
        
        // Top left
        mv.position[0] = v0.position[0] + nx;
        mv.position[1] = v0.position[1] + ny;
        mv.texcoord[0] = 0.0f;
        mv.texcoord[1] = 0.0f;
        std::memcpy(mv.color, v0.color, sizeof(float) * 4);
        m_vertexData.push_back(mv);
        
        // Bottom left
        mv.position[0] = v0.position[0] - nx;
        mv.position[1] = v0.position[1] - ny;
        mv.texcoord[0] = 0.0f;
        mv.texcoord[1] = 1.0f;
        m_vertexData.push_back(mv);
        
        // Top right
        mv.position[0] = v1.position[0] + nx;
        mv.position[1] = v1.position[1] + ny;
        mv.texcoord[0] = 1.0f;
        mv.texcoord[1] = 0.0f;
        std::memcpy(mv.color, v1.color, sizeof(float) * 4);
        m_vertexData.push_back(mv);
        
        // Bottom right
        mv.position[0] = v1.position[0] - nx;
        mv.position[1] = v1.position[1] - ny;
        mv.texcoord[0] = 1.0f;
        mv.texcoord[1] = 1.0f;
        m_vertexData.push_back(mv);
    }
    
    if (m_vertexData.empty()) {
        return;
    }
    
    // Update vertex buffer
    UpdateVertexBuffer(reinterpret_cast<const Vertex*>(m_vertexData.data()), m_vertexData.size());
    
    // Set line pipeline and draw
    wgpuRenderPassEncoderSetPipeline(m_renderPassEncoder, m_linePipeline);
    wgpuRenderPassEncoderSetVertexBuffer(m_renderPassEncoder, 0, m_vertexBuffer, 0, m_vertexData.size() * sizeof(WGPUVertex));
    wgpuRenderPassEncoderDraw(m_renderPassEncoder, m_vertexData.size(), 1, 0, 0);
    
    // Restore triangle pipeline
    wgpuRenderPassEncoderSetPipeline(m_renderPassEncoder, m_trianglePipeline);
}

bool WGPUCommandBuffer::CreateVertexBuffer() {
    const size_t initialSize = 1024 * sizeof(WGPUVertex);
    
    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.size = initialSize;
    bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    bufferDesc.mappedAtCreation = false;
    
    m_vertexBuffer = wgpuDeviceCreateBuffer(m_device->GetWGPUDevice(), &bufferDesc);
    if (!m_vertexBuffer) {
        return false;
    }
    
    m_vertexBufferCapacity = initialSize;
    return true;
}

void WGPUCommandBuffer::UpdateVertexBuffer(const Vertex* vertices, uint32_t vertexCount) {
    size_t requiredSize = vertexCount * sizeof(WGPUVertex);
    
    // Create or resize vertex buffer if needed
    if (!m_vertexBuffer || requiredSize > m_vertexBufferCapacity) {
        if (m_vertexBuffer) {
            wgpuBufferDestroy(m_vertexBuffer);
        }
        
        size_t newSize = std::max(requiredSize, m_vertexBufferCapacity * 2);
        WGPUBufferDescriptor bufferDesc = {};
        bufferDesc.size = newSize;
        bufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        bufferDesc.mappedAtCreation = false;
        
        m_vertexBuffer = wgpuDeviceCreateBuffer(m_device->GetWGPUDevice(), &bufferDesc);
        m_vertexBufferCapacity = newSize;
    }
    
    // Update buffer data
    wgpuQueueWriteBuffer(m_device->GetQueue(), m_vertexBuffer, 0, m_vertexData.data(), requiredSize);
}

WGPURenderPipelineRef WGPUCommandBuffer::GetPipelineForBlendMode(BlendMode mode) {
    switch (mode) {
        case BlendMode::None:
            return m_trianglePipeline;
        case BlendMode::Alpha:
            return m_alphaPipeline;
        case BlendMode::Additive:
            return m_additivePipeline;
        case BlendMode::Multiply:
            return m_multiplyPipeline;
        case BlendMode::Screen:
            return m_screenPipeline;
        default:
            return m_trianglePipeline;
    }
}

bool WGPUCommandBuffer::CreatePipelines() {
    // Create bind group layout
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
    m_bindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_device->GetWGPUDevice(), &bindGroupLayoutDesc);
    if (!m_bindGroupLayout) {
        return false;
    }
    
    // Create pipeline layout
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &m_bindGroupLayout;
    
    WGPUPipelineLayoutRef pipelineLayout = wgpuDeviceCreatePipelineLayout(m_device->GetWGPUDevice(), &pipelineLayoutDesc);
    if (!pipelineLayout) {
        return false;
    }
    
    // Create vertex state
    WGPUVertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.arrayStride = sizeof(WGPUVertex);
    vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
    vertexBufferLayout.attributeCount = 3;
    
    WGPUVertexAttribute attributes[3] = {};
    // Position
    attributes[0].format = WGPUVertexFormat_Float32x2;
    attributes[0].offset = offsetof(WGPUVertex, position);
    attributes[0].shaderLocation = 0;
    // Texcoord
    attributes[1].format = WGPUVertexFormat_Float32x2;
    attributes[1].offset = offsetof(WGPUVertex, texcoord);
    attributes[1].shaderLocation = 1;
    // Color
    attributes[2].format = WGPUVertexFormat_Float32x4;
    attributes[2].offset = offsetof(WGPUVertex, color);
    attributes[2].shaderLocation = 2;
    
    vertexBufferLayout.attributes = attributes;
    
    WGPUVertexState vertexState = {};
    vertexState.module = m_device->GetShaderModule();
    vertexState.entryPoint = "vertex_main";
    vertexState.bufferCount = 1;
    vertexState.buffers = &vertexBufferLayout;
    
    // Create fragment state
    WGPUFragmentState fragmentState = {};
    fragmentState.module = m_device->GetShaderModule();
    fragmentState.entryPoint = "fragment_main";
    fragmentState.targetCount = 1;
    
    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_RGBA8Unorm;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    
    fragmentState.targets = &colorTarget;
    
    // Create pipeline descriptor
    WGPURenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDesc.primitive.cullMode = WGPUCullMode_None;
    
    // Create triangle pipeline
    m_trianglePipeline = wgpuDeviceCreateRenderPipeline(m_device->GetWGPUDevice(), &pipelineDesc);
    if (!m_trianglePipeline) {
        wgpuPipelineLayoutRelease(pipelineLayout);
        return false;
    }
    
    // Create line pipeline
    fragmentState.entryPoint = "fragment_line";
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
    
    m_linePipeline = wgpuDeviceCreateRenderPipeline(m_device->GetWGPUDevice(), &pipelineDesc);
    if (!m_linePipeline) {
        wgpuPipelineLayoutRelease(pipelineLayout);
        return false;
    }
    
    // Create blend mode pipelines
    fragmentState.entryPoint = "fragment_main";
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    
    // Alpha blend
    colorTarget.blend = &GetWGPUBlendState(BlendMode::Alpha);
    m_alphaPipeline = wgpuDeviceCreateRenderPipeline(m_device->GetWGPUDevice(), &pipelineDesc);
    
    // Additive blend
    colorTarget.blend = &GetWGPUBlendState(BlendMode::Additive);
    m_additivePipeline = wgpuDeviceCreateRenderPipeline(m_device->GetWGPUDevice(), &pipelineDesc);
    
    // Multiply blend
    colorTarget.blend = &GetWGPUBlendState(BlendMode::Multiply);
    m_multiplyPipeline = wgpuDeviceCreateRenderPipeline(m_device->GetWGPUDevice(), &pipelineDesc);
    
    // Screen blend
    colorTarget.blend = &GetWGPUBlendState(BlendMode::Screen);
    m_screenPipeline = wgpuDeviceCreateRenderPipeline(m_device->GetWGPUDevice(), &pipelineDesc);
    
    wgpuPipelineLayoutRelease(pipelineLayout);
    return true;
}

void WGPUCommandBuffer::DestroyPipelines() {
    if (m_screenPipeline) wgpuRenderPipelineRelease(m_screenPipeline);
    if (m_multiplyPipeline) wgpuRenderPipelineRelease(m_multiplyPipeline);
    if (m_additivePipeline) wgpuRenderPipelineRelease(m_additivePipeline);
    if (m_alphaPipeline) wgpuRenderPipelineRelease(m_alphaPipeline);
    if (m_linePipeline) wgpuRenderPipelineRelease(m_linePipeline);
    if (m_trianglePipeline) wgpuRenderPipelineRelease(m_trianglePipeline);
}

} // namespace wgpu
} // namespace labfont
