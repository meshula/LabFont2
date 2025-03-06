#ifndef LABFONT_WGPU_BACKEND_H
#define LABFONT_WGPU_BACKEND_H

#include "core/backend.h"

// Forward declarations for WebGPU types to avoid include errors during development
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
  #include <webgpu/webgpu.h>
#else
  // Forward declarations for development without Emscripten
  typedef struct WGPUShaderModuleImpl* WGPUShaderModule;
  typedef struct WGPURenderPipelineImpl* WGPURenderPipeline;
  typedef struct WGPUBindGroupLayoutImpl* WGPUBindGroupLayout;
  typedef struct WGPUTextureImpl* WGPUTexture;
  typedef struct WGPUTextureViewImpl* WGPUTextureView;
  typedef struct WGPURenderPassDescriptorImpl* WGPURenderPassDescriptor;
#endif

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

// Forward declarations of WebGPU-specific classes
// These would be fully implemented in a real build with WebGPU headers
class WebGPUTexture : public Texture {
public:
    WebGPUTexture(const WebGPUDevice* device, const TextureDesc& desc) 
        : m_width(desc.width), m_height(desc.height), m_format(desc.format),
          m_renderTarget(false), m_readback(false), m_texture(nullptr), m_textureView(nullptr) {}
    ~WebGPUTexture() override {}
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool IsRenderTarget() const override { return m_renderTarget; }
    bool SupportsReadback() const override { return m_readback; }
    
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
    WebGPURenderTarget(const WebGPUDevice* device, const RenderTargetDesc& desc)
        : m_width(desc.width), m_height(desc.height), m_format(desc.format),
          m_hasDepth(desc.hasDepth), m_renderPassDesc(nullptr) {}
    ~WebGPURenderTarget() override {}
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool HasDepth() const override { return m_hasDepth; }
    
    Texture* GetColorTexture() override { return m_colorTexture.get(); }
    Texture* GetDepthTexture() override { return m_depthTexture.get(); }
    
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
