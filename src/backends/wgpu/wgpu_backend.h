#ifndef LABFONT_WGPU_BACKEND_H
#define LABFONT_WGPU_BACKEND_H

#include "core/backend.h"
#include "wgpu_device.h"
#include "wgpu_texture.h"
#include "wgpu_render_target.h"
#include "wgpu_command_buffer.h"

namespace labfont {

class WGPUBackend : public Backend {
public:
    WGPUBackend() : m_device(std::make_unique<WebGPUDevice>()) {}
    ~WGPUBackend() override;
    
    lab_result Initialize(uint32_t width, uint32_t height) override;
    lab_result Resize(uint32_t width, uint32_t height) override;
    
    // Texture management
    lab_result CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) override;
    lab_result UpdateTexture(Texture* texture, const void* data, size_t size) override;
    lab_result ReadbackTexture(Texture* texture, void* data, size_t size) override;
    
    // Render target management
    lab_result CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) override;
    lab_result SetRenderTarget(RenderTarget* target) override;
    
    // Draw command submission
    lab_result BeginFrame() override;
    lab_result SubmitCommands(const std::vector<DrawCommand>& commands) override;
    lab_result EndFrame() override;
    
    // Resource cleanup
    void DestroyTexture(Texture* texture) override;
    void DestroyRenderTarget(RenderTarget* target) override;
    
    // Memory management
    size_t GetTextureMemoryUsage() const override { return 0; }
    size_t GetTotalMemoryUsage() const override { return 0; }
    
    // Backend capabilities
    bool SupportsTextureFormat(TextureFormat format) const override;
    bool SupportsBlendMode(BlendMode mode) const override;
    uint32_t GetMaxTextureSize() const override { return 8192; }
    
    // WGPU-specific getters
    WebGPUDevice* GetDevice() { return m_device.get(); }
    
private:
    std::unique_ptr<WebGPUDevice> m_device;
    
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    WGPUShaderModule m_shaderModule = nullptr;
    WGPURenderPipeline m_trianglePipeline = nullptr;
    WGPURenderPipeline m_linePipeline = nullptr;
    WGPUBindGroupLayout m_bindGroupLayout = nullptr;
#else
    void* m_shaderModule = nullptr;
    void* m_trianglePipeline = nullptr;
    void* m_linePipeline = nullptr;
    void* m_bindGroupLayout = nullptr;
#endif
    
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    
    RenderTarget* m_currentRenderTarget = nullptr;
    std::unique_ptr<WebGPUCommandBuffer> m_currentCommandBuffer;
    std::vector<std::shared_ptr<Texture>> m_textures;
    std::vector<std::shared_ptr<RenderTarget>> m_renderTargets;
};

} // namespace labfont

#endif // LABFONT_WGPU_BACKEND_H
