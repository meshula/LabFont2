#include "wgpu_backend.h"

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
  #include <emscripten/emscripten.h>
#endif

namespace labfont {

WGPUBackend::~WGPUBackend() {
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    if (m_shaderModule) wgpuShaderModuleRelease(m_shaderModule);
    if (m_trianglePipeline) wgpuRenderPipelineRelease(m_trianglePipeline);
    if (m_linePipeline) wgpuRenderPipelineRelease(m_linePipeline);
    if (m_bindGroupLayout) wgpuBindGroupLayoutRelease(m_bindGroupLayout);
#endif
}

lab_result WGPUBackend::Initialize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    // Get WebGPU device from Emscripten
    m_device->device = emscripten_webgpu_get_device();
    if (!m_device->device) {
        return LAB_RESULT_DEVICE_INITIALIZATION_FAILED;
    }
    
    // Get queue
    m_device->queue = wgpuDeviceGetQueue(m_device->device);
    if (!m_device->queue) {
        return LAB_RESULT_DEVICE_INITIALIZATION_FAILED;
    }
    
    // Create shader module and pipelines
    // This would be implemented with actual WebGPU calls in a real build
#else
    // Stub implementation for development without Emscripten
    return LAB_RESULT_UNSUPPORTED_BACKEND;
#endif

    return LAB_RESULT_OK;
}

lab_result WGPUBackend::Resize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return LAB_RESULT_OK;
}

lab_result WGPUBackend::CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) {
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    auto texture = std::make_shared<WebGPUTexture>(m_device.get(), desc);
    if (!texture->GetWGPUTexture()) {
        return LAB_RESULT_TEXTURE_CREATION_FAILED;
    }
    
    out_texture = texture;
    m_textures.push_back(texture);
    return LAB_RESULT_OK;
#else
    return LAB_RESULT_UNSUPPORTED_BACKEND;
#endif
}

lab_result WGPUBackend::UpdateTexture(Texture* texture, const void* data, size_t size) {
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    auto webgpuTexture = static_cast<WebGPUTexture*>(texture);
    // Implementation would go here
    return LAB_RESULT_OK;
#else
    return LAB_RESULT_UNSUPPORTED_BACKEND;
#endif
}

lab_result WGPUBackend::ReadbackTexture(Texture* texture, void* data, size_t size) {
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    auto webgpuTexture = static_cast<WebGPUTexture*>(texture);
    // Implementation would go here
    return LAB_RESULT_OK;
#else
    return LAB_RESULT_UNSUPPORTED_BACKEND;
#endif
}

lab_result WGPUBackend::CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) {
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    auto target = std::make_shared<WebGPURenderTarget>(m_device.get(), desc);
    if (!target->GetRenderPassDescriptor()) {
        return LAB_RESULT_RENDERTARGET_INITIALIZATION_FAILED;
    }
    
    out_target = target;
    m_renderTargets.push_back(target);
    return LAB_RESULT_OK;
#else
    return LAB_RESULT_UNSUPPORTED_BACKEND;
#endif
}

lab_result WGPUBackend::SetRenderTarget(RenderTarget* target) {
    m_currentRenderTarget = target;
    return LAB_RESULT_OK;
}

lab_result WGPUBackend::BeginFrame() {
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    m_currentCommandBuffer = std::make_unique<WebGPUCommandBuffer>(m_device.get());
    if (!m_currentCommandBuffer->Begin()) {
        return LAB_RESULT_COMMAND_BUFFER_INITIALIZATION_FAILED;
    }
    return LAB_RESULT_OK;
#else
    return LAB_RESULT_UNSUPPORTED_BACKEND;
#endif
}

lab_result WGPUBackend::SubmitCommands(const std::vector<DrawCommand>& commands) {
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    if (!m_currentRenderTarget) {
        return LAB_RESULT_STATE_NO_RENDER_TARGET_SET;
    }
    
    auto webgpuTarget = static_cast<WebGPURenderTarget*>(m_currentRenderTarget);
    if (!m_currentCommandBuffer->BeginRenderPass(webgpuTarget)) {
        return LAB_RESULT_COMMAND_ENCODER_INITIALIZATION_FAILED;
    }
    
    for (const auto& cmd : commands) {
        switch (cmd.type) {
            case DrawCommandType::Clear: {
                m_currentCommandBuffer->Clear(cmd.clear.color);
                break;
            }
            
            case DrawCommandType::DrawTriangles: {
                const auto& params = cmd.triangles;
                // Convert lab_vertex_2TC to Vertex
                std::vector<Vertex> vertices;
                vertices.reserve(params.vertexCount);
                for (uint32_t i = 0; i < params.vertexCount; ++i) {
                    vertices.push_back(Vertex(params.vertices[i]));
                }
                m_currentCommandBuffer->DrawTriangles(vertices.data(), params.vertexCount);
                break;
            }
            
            case DrawCommandType::DrawLines: {
                const auto& params = cmd.lines;
                // Convert lab_vertex_2TC to Vertex
                std::vector<Vertex> vertices;
                vertices.reserve(params.vertexCount);
                for (uint32_t i = 0; i < params.vertexCount; ++i) {
                    vertices.push_back(Vertex(params.vertices[i]));
                }
                m_currentCommandBuffer->DrawLines(vertices.data(), params.vertexCount, params.lineWidth);
                break;
            }
            
            case DrawCommandType::SetBlendMode: {
                m_currentCommandBuffer->SetBlendMode(cmd.blend.mode);
                break;
            }
            
            case DrawCommandType::SetScissor: {
                const auto& params = cmd.scissor;
                m_currentCommandBuffer->SetScissorRect(
                    params.x, params.y,
                    params.width, params.height
                );
                break;
            }
            
            case DrawCommandType::SetViewport: {
                const auto& params = cmd.viewport;
                m_currentCommandBuffer->SetViewport(
                    params.x, params.y,
                    params.width, params.height
                );
                break;
            }
        }
    }
    
    m_currentCommandBuffer->EndRenderPass();
    return LAB_RESULT_OK;
#else
    return LAB_RESULT_UNSUPPORTED_BACKEND;
#endif
}

lab_result WGPUBackend::EndFrame() {
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    if (!m_currentCommandBuffer->End()) {
        return LAB_RESULT_INVALID_COMMAND_BUFFER;
    }
    m_currentCommandBuffer.reset();
    return LAB_RESULT_OK;
#else
    return LAB_RESULT_UNSUPPORTED_BACKEND;
#endif
}

void WGPUBackend::DestroyTexture(Texture* texture) {
    for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
        if (it->get() == texture) {
            m_textures.erase(it);
            break;
        }
    }
}

void WGPUBackend::DestroyRenderTarget(RenderTarget* target) {
    for (auto it = m_renderTargets.begin(); it != m_renderTargets.end(); ++it) {
        if (it->get() == target) {
            m_renderTargets.erase(it);
            break;
        }
    }
}

bool WGPUBackend::SupportsTextureFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            return true;
        default:
            return false;
    }
}

bool WGPUBackend::SupportsBlendMode(BlendMode mode) const {
    return true; // All blend modes supported
}

} // namespace labfont
