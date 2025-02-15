#ifndef LABFONT_WGPU_BACKEND_H
#define LABFONT_WGPU_BACKEND_H

#include "core/backend.h"
#include "wgpu_types.h"
#include <memory>
#include <vector>

namespace labfont {
namespace wgpu {

// Forward declarations
class WGPUCommandBuffer;

// WebGPU-based texture implementation
class WGPUTexture : public Texture {
public:
    WGPUTexture(WGPUDevice* device, const TextureDesc& desc);
    ~WGPUTexture() override;
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool IsRenderTarget() const override { return m_renderTarget; }
    bool SupportsReadback() const override { return m_readback; }
    
    // WebGPU-specific methods
    WGPUTextureRef GetWGPUTexture() const { return m_texture; }
    WGPUTextureViewRef GetWGPUTextureView() const { return m_textureView; }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_renderTarget;
    bool m_readback;
    WGPUTextureRef m_texture;
    WGPUTextureViewRef m_textureView;
    WGPUDevice* m_device;
};

// WebGPU-based render target implementation
class WGPURenderTarget : public RenderTarget {
public:
    WGPURenderTarget(WGPUDevice* device, const RenderTargetDesc& desc);
    ~WGPURenderTarget() override;
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool HasDepth() const override { return m_hasDepth; }
    Texture* GetColorTexture() override { return m_colorTexture.get(); }
    Texture* GetDepthTexture() override { return m_depthTexture.get(); }
    
    // WebGPU-specific methods
    const RenderPassDesc& GetRenderPassDesc() const { return m_renderPassDesc; }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_hasDepth;
    std::shared_ptr<WGPUTexture> m_colorTexture;
    std::shared_ptr<WGPUTexture> m_depthTexture;
    RenderPassDesc m_renderPassDesc;
    WGPUDevice* m_device;
};

// WebGPU device wrapper
class WGPUDevice {
public:
    WGPUDevice();
    ~WGPUDevice();
    
    bool Initialize();
    
    WGPUDeviceRef GetWGPUDevice() const { return m_device; }
    WGPUQueueRef GetQueue() const { return m_queue; }
    
    // Pipeline state objects
    WGPURenderPipelineRef GetTrianglePipeline() const { return m_trianglePipeline; }
    WGPURenderPipelineRef GetLinePipeline() const { return m_linePipeline; }
    
    // Bind group layout
    WGPUBindGroupLayoutRef GetBindGroupLayout() const { return m_bindGroupLayout; }
    
private:
    bool CreatePipelineStates();
    bool LoadShaders();
    
    WGPUDeviceRef m_device;
    WGPUQueueRef m_queue;
    WGUShaderModuleRef m_shaderModule;
    WGPURenderPipelineRef m_trianglePipeline;
    WGPURenderPipelineRef m_linePipeline;
    WGPUBindGroupLayoutRef m_bindGroupLayout;
};

// WebGPU-based backend implementation
class WGPUBackend : public Backend {
public:
    WGPUBackend();
    ~WGPUBackend() override;
    
    lab_result Initialize(uint32_t width, uint32_t height) override;
    lab_result Resize(uint32_t width, uint32_t height) override;
    
    lab_result CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) override;
    lab_result UpdateTexture(Texture* texture, const void* data, size_t size) override;
    lab_result ReadbackTexture(Texture* texture, void* data, size_t size) override;
    
    lab_result CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) override;
    lab_result SetRenderTarget(RenderTarget* target) override;
    
    lab_result BeginFrame() override;
    lab_result SubmitCommands(const std::vector<DrawCommand>& commands) override;
    lab_result EndFrame() override;
    
    void DestroyTexture(Texture* texture) override;
    void DestroyRenderTarget(RenderTarget* target) override;
    
    size_t GetTextureMemoryUsage() const override;
    size_t GetTotalMemoryUsage() const override;
    
    bool SupportsTextureFormat(TextureFormat format) const override;
    bool SupportsBlendMode(BlendMode mode) const override;
    uint32_t GetMaxTextureSize() const override;
    
private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::unique_ptr<WGPUDevice> m_device;
    std::vector<std::shared_ptr<Texture>> m_textures;
    std::vector<std::shared_ptr<RenderTarget>> m_renderTargets;
    RenderTarget* m_currentRenderTarget = nullptr;
    BlendMode m_currentBlendMode = BlendMode::None;
    std::unique_ptr<WGPUCommandBuffer> m_currentCommandBuffer;
};

} // namespace wgpu
} // namespace labfont

#endif // LABFONT_WGPU_BACKEND_H
