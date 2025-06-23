#import "metal_command_buffer.h"
#import "metal_backend.h"
#import "core/resource.h"
#import <Metal/Metal.h>
#include <iostream>

namespace labfont {
namespace metal {

MetalCommandBuffer::MetalCommandBuffer(MetalDevice* device)
    : m_device(device)
    , m_commandBuffer(nil)
    , m_renderEncoder(nil)
    , m_vertexBuffer(nil)
    , m_vertexBufferCapacity(0)
    , m_currentBlendMode(BlendMode::None)
    , m_currentTexture(nullptr)
    , m_inRenderPass(false)
    , m_currentDrawMode(DrawMode::Triangles)
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
        std::cerr << "Error: Failed to create command buffer!\n";
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

void ValidateRenderPassDescriptor(MTLRenderPassDescriptor *desc) {
    if (!desc) {
        std::cerr << "Error: Render pass descriptor is null.\n";
        return;
    }

    bool hasValidAttachment = false;

    const NSUInteger MTLMaxColorAttachments = 4; // only checking four
    for (NSUInteger i = 0; i < MTLMaxColorAttachments; ++i) {
        auto *colorAttachment = desc.colorAttachments[i];
        if (colorAttachment && colorAttachment.texture) {
            hasValidAttachment = true;
            break;
        }
    }

    if (!hasValidAttachment && !desc.depthAttachment.texture && !desc.stencilAttachment.texture) {
        std::cerr << "Error: No valid color, depth, or stencil attachments.\n";
    }

    for (NSUInteger i = 0; i < MTLMaxColorAttachments; ++i) {
        auto *colorAttachment = desc.colorAttachments[i];
        if (colorAttachment && colorAttachment.texture) {
            std::cout << "Color Attachment " << i << " has a valid texture.\n";
        } else {
            std::cerr << "Warning: Color Attachment " << i << " is missing a texture.\n";
        }
    }

    if (desc.depthAttachment.texture) {
        std::cout << "Depth attachment is set.\n";
    } else {
        std::cerr << "Warning: Depth attachment is missing.\n";
    }

    if (desc.stencilAttachment.texture) {
        std::cout << "Stencil attachment is set.\n";
    } else {
        std::cerr << "Warning: Stencil attachment is missing.\n";
    }
}

void ValidateCommandBuffer(id<MTLCommandBuffer> commandBuffer) {
    if (!commandBuffer) {
        std::cerr << "Error: Command buffer is null.\n";
        return;
    }

    switch (commandBuffer.status) {
        case MTLCommandBufferStatusNotEnqueued:
            std::cerr << "Warning: Command buffer not enqueued.\n";
            break;
        case MTLCommandBufferStatusEnqueued:
            std::cout << "Command buffer enqueued.\n";
            break;
        case MTLCommandBufferStatusCommitted:
            std::cout << "Command buffer committed.\n";
            break;
        case MTLCommandBufferStatusScheduled:
            std::cout << "Command buffer scheduled.\n";
            break;
        case MTLCommandBufferStatusCompleted:
            std::cout << "Command buffer completed successfully.\n";
            break;
        case MTLCommandBufferStatusError:
            std::cerr << "Error: Command buffer execution failed.\n";
            if (commandBuffer.error) {
                std::cerr << "Metal Error: " << commandBuffer.error.localizedDescription.UTF8String << "\n";
            }
            break;
    }
}

lab_result MetalCommandBuffer::BeginRenderPass(MetalRenderTarget* target) {
    if (m_inRenderPass) {
        EndRenderPass();
    }
    
    MTLRenderPassDescriptor* renderPass = target->GetRenderPassDescriptor();
    if (!renderPass || !m_commandBuffer) {
        return LAB_RESULT_INVALID_BUFFER;
    }

    static bool validate = false;
    if (validate) {
        ValidateRenderPassDescriptor(renderPass);
        ValidateCommandBuffer(m_commandBuffer);
    }
    
    m_renderEncoder = [m_commandBuffer renderCommandEncoderWithDescriptor:renderPass];
    if (!m_renderEncoder) {
        return LAB_RESULT_COMMAND_ENCODER_INITIALIZATION_FAILED;
    }
    
    m_inRenderPass = true;
    return LAB_RESULT_OK;
}

void MetalCommandBuffer::EndRenderPass() {
    Flush(m_currentDrawMode);
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
    if (vertexCount == 0) {
        return;
    }

    if (m_currentDrawMode != DrawMode::Triangles) {
        Flush(m_currentDrawMode);
        m_currentDrawMode = DrawMode::Triangles;
    }

    // ensure memcpy is safe
    static_assert(sizeof(Vertex::position) == sizeof(MetalVertex::position), "Position size mismatch");
    static_assert(sizeof(Vertex::texcoord) == sizeof(MetalVertex::texcoord), "Texcoord size mismatch");
    static_assert(sizeof(Vertex::color) == sizeof(MetalVertex::color), "Color size mismatch");
    static_assert(sizeof(Vertex) == sizeof(MetalVertex), "Vertex and MetalVertex sizes must match.");
    static_assert(alignof(Vertex) == alignof(MetalVertex), "Vertex and MetalVertex alignment must match.");

    size_t offset = m_vertexData.size();
    if (offset + vertexCount > m_vertexData.capacity()) {
        m_vertexData.reserve(std::max(offset * 2, m_vertexData.size() + vertexCount));
    }
    m_vertexData.resize(offset + vertexCount);
    std::memcpy(m_vertexData.data() + offset, vertices, vertexCount * sizeof(Vertex));
}

void MetalCommandBuffer::DrawLines(const Vertex* vertices, uint32_t vertexCount, float lineWidth) {
    if (vertexCount == 0) return;

    if (m_currentDrawMode != DrawMode::Lines) {
        Flush(m_currentDrawMode);
        m_currentDrawMode = DrawMode::Lines;
    }

    // Reserve space to avoid frequent reallocations
    size_t additionalVertices = (vertexCount / 2) * 4; // 4 vertices per line segment
    if (m_vertexData.capacity() < m_vertexData.size() + additionalVertices) {
        m_vertexData.reserve(std::max(m_vertexData.size() * 2, m_vertexData.size() + additionalVertices));
    }

    for (uint32_t i = 0; i + 1 < vertexCount; i += 2) {
        const Vertex& v0 = vertices[i];
        const Vertex& v1 = vertices[i + 1];

        // Compute direction and normal
        float dx = v1.position[0] - v0.position[0];
        float dy = v1.position[1] - v0.position[1];
        float length = std::sqrt(dx * dx + dy * dy);

        if (length < 1e-6f) continue; // Skip degenerate lines

        float nx = -dy / length * (lineWidth * 0.5f);
        float ny = dx / length * (lineWidth * 0.5f);

        // Append four vertices for the thickened line quad
        m_vertexData.emplace_back(v0.position[0] + nx, v0.position[1] + ny, 0.0f, 0.0f, v0.color);
        m_vertexData.emplace_back(v0.position[0] - nx, v0.position[1] - ny, 0.0f, 1.0f, v0.color);
        m_vertexData.emplace_back(v1.position[0] + nx, v1.position[1] + ny, 1.0f, 0.0f, v1.color);
        m_vertexData.emplace_back(v1.position[0] - nx, v1.position[1] - ny, 1.0f, 1.0f, v1.color);
    }
}

void MetalCommandBuffer::BindTexture(lab_texture texture) {
    if (m_currentTexture == texture) {
        return;
    }
    // finish drawing with previously bound textures
    Flush(m_currentDrawMode);
    m_currentTexture = texture;
}

void MetalCommandBuffer::Flush(DrawMode mode) {
    if (m_vertexData.empty() || (m_currentDrawMode == DrawMode::None)) {
        return;
    }

    if (m_vertexBufferCapacity < m_vertexData.size()) {
        // Resize Metal buffer if necessary
        m_vertexBufferCapacity = m_vertexData.size();
        CreateVertexBuffer();
    }

    UpdateVertexBuffer();

    // Set line pipeline and draw
    if (mode == DrawMode::Triangles) {
        if (m_currentTexture) {
            [m_renderEncoder setRenderPipelineState:m_device->GetTexturedTrianglePipeline()];
        }
        else {
            [m_renderEncoder setRenderPipelineState:m_device->GetTrianglePipeline()];
        }
    } else if (mode == DrawMode::Lines) {
        [m_renderEncoder setRenderPipelineState:m_device->GetLinePipeline()];
    }

    if (m_currentTexture) {
        auto textureResource = reinterpret_cast<labfont::TextureResource*>(m_currentTexture);
        if (!textureResource->IsValid()) {
            return; // Exit early if texture is invalid
        }
        if (textureResource->texture) {
            MetalTexture* mt = dynamic_cast<MetalTexture*>(textureResource->texture.get());
            if (mt) {
                [m_renderEncoder setFragmentTexture:mt->GetMTLTexture() atIndex:0];
            }
        }
    }
    
    [m_renderEncoder setVertexBuffer:m_vertexBuffer offset:0 atIndex:0];
    [m_renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
                      vertexStart:0
                      vertexCount:m_vertexData.size()];

    m_vertexData.clear();
    m_currentDrawMode = DrawMode::None;
}


bool MetalCommandBuffer::CreateVertexBuffer() {
    const size_t initialCount = 1024;
    const size_t initialSize = initialCount * sizeof(MetalVertex);
    m_vertexData.reserve(initialCount);

    MTLResourceOptions options = MTLResourceStorageModeShared | MTLResourceCPUCacheModeWriteCombined;
    m_vertexBuffer = [m_device->GetMTLDevice() newBufferWithLength:initialSize
                                                          options:options];
    
    if (!m_vertexBuffer) {
        return false;
    }
    
    m_vertexBufferCapacity = initialSize;
    return true;
}

void MetalCommandBuffer::UpdateVertexBuffer() {
    size_t requiredSize = m_vertexData.size() * sizeof(MetalVertex);

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
