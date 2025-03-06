#include "wgpu_backend.h"
#include "wgpu_device.h"

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
  #include <emscripten/emscripten.h>
#endif

namespace labfont {

WGPUBackend::~WGPUBackend() {
    // In a real implementation with WebGPU headers:
    // if (m_shaderModule) wgpuShaderModuleRelease(m_shaderModule);
    // if (m_trianglePipeline) wgpuRenderPipelineRelease(m_trianglePipeline);
    // if (m_linePipeline) wgpuRenderPipelineRelease(m_linePipeline);
    // if (m_bindGroupLayout) wgpuBindGroupLayoutRelease(m_bindGroupLayout);
}

lab_result WGPUBackend::Initialize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    // Get WebGPU device from Emscripten
    m_device->device = emscripten_webgpu_get_device();
    if (!m_device->device) {
        return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "Failed to get WebGPU device");
    }
    
    // Get queue
    m_device->queue = wgpuDeviceGetQueue(m_device->device);
    if (!m_device->queue) {
        return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "Failed to get WebGPU queue");
    }
    
    // Create shader module and pipelines
    // This would be implemented with actual WebGPU calls in a real build
#else
    // Stub implementation for development without Emscripten
    return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "WebGPU backend requires Emscripten");
#endif

    return lab_result(LAB_ERROR_NONE);
}

lab_result WGPUBackend::Resize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return lab_result(LAB_ERROR_NONE);
}

// Implement the required virtual methods with stub implementations
lab_result WGPUBackend::CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) {
    return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "WebGPU backend not fully implemented");
}

lab_result WGPUBackend::UpdateTexture(Texture* texture, const void* data, size_t size) {
    return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "WebGPU backend not fully implemented");
}

lab_result WGPUBackend::ReadbackTexture(Texture* texture, void* data, size_t size) {
    return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "WebGPU backend not fully implemented");
}

lab_result WGPUBackend::CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) {
    return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "WebGPU backend not fully implemented");
}

lab_result WGPUBackend::SetRenderTarget(RenderTarget* target) {
    return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "WebGPU backend not fully implemented");
}

lab_result WGPUBackend::BeginFrame() {
    return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "WebGPU backend not fully implemented");
}

lab_result WGPUBackend::SubmitCommands(const std::vector<DrawCommand>& commands) {
    return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "WebGPU backend not fully implemented");
}

lab_result WGPUBackend::EndFrame() {
    return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "WebGPU backend not fully implemented");
}

void WGPUBackend::DestroyTexture(Texture* texture) {
    // Stub implementation
}

void WGPUBackend::DestroyRenderTarget(RenderTarget* target) {
    // Stub implementation
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
