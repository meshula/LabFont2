#import "metal_command_buffer.h"
#import "metal_backend.h"
#import <Metal/Metal.h>

namespace labfont {
namespace metal {

MetalCommandBuffer::MetalCommandBuffer(MetalDevice* device)
    : m_device(device)
    , m_commandBuffer(nil)
    , m_renderEncoder(nil)
    , m_vertexBuffer(nil)
    , m_vertexBufferCapacity(0)
    , m_currentBlendMode(BlendMode::None)
    , m_inRenderPass(false)
{
    CreateVertexBuffer();
}

MetalCommandBuffer::~MetalCommandBuffer() {
    if (m_vertexBuffer) {
        [m_vertexBuffer release];
    }
}

bool MetalCommandBuffer::Begin() {
    m_commandBuffer = [m_device->GetCommandQueue() commandBuffer];
    if (!m_commandBuffer) {
        return false;
    }
    return true;
}

bool MetalCommandBuffer::End() {
    if (m_inRenderPass) {
        EndRenderPass();
    }
    
    [m_commandBuffer commit];
    [m_commandBuffer waitUntilCompleted];
    m_commandBuffer = nil;
    return true;
}

bool MetalCommandBuffer::BeginRenderPass(MetalRenderTarget* target) {
    if (m_inRenderPass) {
        EndRenderPass();
    }
    
    MTLRenderPassDescriptor* renderPass = target->GetRenderPassDescriptor();
    if (!renderPass) {
        return false;
    }
    
    m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:renderPass];
    if (!m_renderEncoder) {
        return false;
    }
    
    m_inRenderPass = true;
    return true;
}

void MetalCommandBuffer::EndRenderPass() {
    if (m_renderEncoder) {
        [m_renderEncoder endEncoding];
        m_renderEncoder = nil;
    }
    m_inRenderPass = false;
}

void MetalCommandBuffer::SetBlendMode(BlendMode mode) {
    m_currentBlendMode = mode;
    
    if (m_renderEncoder) {
        MetalRenderPipelineStateRef pipeline = nil;
        switch (mode) {
            case BlendMode::None:
                pipeline = m_device->GetTrianglePipeline();
                break;
            case BlendMode::Alpha:
                // TODO: Set alpha blend pipeline
                break;
            case BlendMode::Additive:
                // TODO: Set additive blend pipeline
                break;
            case BlendMode::Multiply:
                // TODO: Set multiply blend pipeline
                break;
            case BlendMode::Screen:
                // TODO: Set screen blend pipeline
                break;
        }
        
        if (pipeline) {
            [m_renderEncoder setRenderPipelineState:pipeline];
        }
    }
}

void MetalCommandBuffer::SetScissorRect(int32_t x, int32_t y, uint32_t width, uint32_t height) {
    if (m_renderEncoder) {
        MTLScissorRect rect = { 
            static_cast<NSUInteger>(x),
            static_cast<NSUInteger>(y),
            static_cast<NSUInteger>(width),
            static_cast<NSUInteger>(height)
        };
        [m_renderEncoder setScissorRect:rect];
    }
}

void MetalCommandBuffer::SetViewport(float x, float y, float width, float height) {
    if (m_renderEncoder) {
        MTLViewport viewport = {
            x, y, width, height,
            0.0, 1.0  // depth range
        };
        [m_renderEncoder setViewport:viewport];
    }
}

void MetalCommandBuffer::Clear(const float color[4]) {
    // Clear is handled through render pass load action
}

void MetalCommandBuffer::DrawTriangles(const Vertex* vertices, uint32_t vertexCount) {
    if (!m_renderEncoder || vertexCount == 0) {
        return;
    }
    
    // Convert vertices to Metal format
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
    
    // Create or update vertex buffer
    if (!m_vertexBuffer) {
        if (!CreateVertexBuffer()) {
            return;
        }
    }
    UpdateVertexBuffer(vertices, vertexCount);
    
    // Set vertex buffer and draw
    [m_renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:0];
    [m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                      vertexStart:0
                      vertexCount:vertexCount];
}

void MetalCommandBuffer::DrawLines(const Vertex* vertices, uint32_t vertexCount, float lineWidth) {
    if (!m_renderEncoder || vertexCount == 0) {
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
        MetalVertex mv;
        
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
    
    // Create or update vertex buffer
    if (!m_vertexBuffer) {
        if (!CreateVertexBuffer()) {
            return;
        }
    }
    UpdateVertexBuffer(reinterpret_cast<const Vertex*>(m_vertexData.data()), m_vertexData.size());
    
    // Set line pipeline and draw
    [m_renderEncoder setRenderPipelineState:m_device->GetLinePipeline()];
    [m_renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:0];
    [m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
                      vertexStart:0
                      vertexCount:m_vertexData.size()];
    
    // Restore triangle pipeline
    [m_renderEncoder setRenderPipelineState:m_device->GetTrianglePipeline()];
}

bool MetalCommandBuffer::CreateVertexBuffer() {
    const size_t initialSize = 1024 * sizeof(MetalVertex);
    
    MTLResourceOptions options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined;
    m_vertexBuffer = [m_device->GetMTLDevice() newBufferWithLength:initialSize
                                                          options:options];
    
    if (!m_vertexBuffer) {
        return false;
    }
    
    m_vertexBufferCapacity = initialSize;
    return true;
}

void MetalCommandBuffer::UpdateVertexBuffer(const Vertex* vertices, uint32_t vertexCount) {
    size_t requiredSize = vertexCount * sizeof(MetalVertex);
    
    // Create or resize vertex buffer if needed
    if (!m_vertexBuffer || requiredSize > m_vertexBufferCapacity) {
        if (m_vertexBuffer) {
            [m_vertexBuffer release];
        }
        
        size_t newSize = std::max(requiredSize, m_vertexBufferCapacity * 2);
        MTLResourceOptions options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined;
        m_vertexBuffer = [m_device->GetMTLDevice() newBufferWithLength:newSize
                                                              options:options];
        m_vertexBufferCapacity = newSize;
    }
    
    // Copy vertex data
    void* data = [m_vertexBuffer contents];
    std::memcpy(data, m_vertexData.data(), requiredSize);
}

} // namespace metal
} // namespace labfont
