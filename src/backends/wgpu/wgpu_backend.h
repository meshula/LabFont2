#ifndef LABFONT_WGPU_BACKEND_H
#define LABFONT_WGPU_BACKEND_H

#include "core/backend.h"
#include "wgpu_types.h"
#include <webgpu/webgpu_cpp.h>

namespace labfont {

class WebGPUDevice;
class WebGPURenderTarget;

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
    WGPUShaderModule m_shaderModule = nullptr;
    WGPURenderPipeline m_trianglePipeline = nullptr;
    WGPURenderPipeline m_linePipeline = nullptr;
    WGPUBindGroupLayout m_bindGroupLayout = nullptr;
    
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

class WebGPUTexture : public Texture {
public:
    WebGPUTexture(const WebGPUDevice* device, const TextureDesc& desc);
    ~WebGPUTexture() override;
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool IsRenderTarget() const override { return m_renderTarget; }
    bool SupportsReadback() const override { return m_readback; }
    
    // WGPU-specific getters
    WGPUTexture GetWGPUTexture() const { return m_texture; }
    WGPUTextureView GetWGPUTextureView() const { return m_textureView; }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_renderTarget;
    bool m_readback;
    
    WGPUTexture m_texture;
    WGPUTextureView m_textureView;
};

class WebGPURenderTarget : public RenderTarget {
public:
    WebGPURenderTarget(const WebGPUDevice* device, const RenderTargetDesc& desc);
    ~WebGPURenderTarget() override;
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool HasDepth() const override { return m_hasDepth; }
    
    Texture* GetColorTexture() override { return m_colorTexture.get(); }
    Texture* GetDepthTexture() override { return m_depthTexture.get(); }
    
    // WGPU-specific getters
    const WGPURenderPassDescriptor* GetRenderPassDesc() const { return m_renderPassDesc; }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_hasDepth;
    
    std::shared_ptr<WebGPUTexture> m_colorTexture;
    std::shared_ptr<WebGPUTexture> m_depthTexture;
    WGPURenderPassDescriptor* m_renderPassDesc;
};

} // namespace labfont

#endif // LABFONT_WGPU_BACKEND_H
