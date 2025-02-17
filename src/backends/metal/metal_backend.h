#ifndef LABFONT_METAL_BACKEND_H
#define LABFONT_METAL_BACKEND_H

#include "core/backend.h"
#include <memory>
#include <vector>

#include "metal_types.h"

namespace labfont {
namespace metal {

// Forward declarations
class MetalDevice;
class MetalCommandBuffer;

// Metal-based texture implementation
class MetalTexture : public Texture {
public:
    MetalTexture(MetalDevice* device, const TextureDesc& desc);
    ~MetalTexture() override;
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool IsRenderTarget() const override { return m_renderTarget; }
    bool SupportsReadback() const override { return m_readback; }
    
    // Metal-specific methods
    MetalTextureRef GetMTLTexture() const { return m_texture; }
    MTLPixelFormat TextureFormatToMTLFormat(TextureFormat format);
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_renderTarget;
    bool m_readback;
    MetalTextureRef m_texture;
    MetalDevice* m_device;
};

// Metal-based render target implementation
class MetalRenderTarget : public RenderTarget {
public:
    MetalRenderTarget(MetalDevice* device, const RenderTargetDesc& desc);
    ~MetalRenderTarget() override;
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool HasDepth() const override { return m_hasDepth; }
    Texture* GetColorTexture() override { return m_colorTexture.get(); }
    Texture* GetDepthTexture() override { return m_depthTexture.get(); }
    
    // Metal-specific methods
    MetalRenderPassDescriptorRef GetRenderPassDescriptor() const { return m_renderPassDescriptor; }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_hasDepth;
    std::shared_ptr<MetalTexture> m_colorTexture;
    std::shared_ptr<MetalTexture> m_depthTexture;
    MetalRenderPassDescriptorRef m_renderPassDescriptor;
    MetalDevice* m_device;
};

// Metal device wrapper
class MetalDevice {
public:
    MetalDevice();
    ~MetalDevice();
    
    bool Initialize();
    
    MetalDeviceRef GetMTLDevice() const { return m_device; }
    MetalCommandQueueRef GetCommandQueue() const { return m_commandQueue; }
    MetalLibraryRef GetShaderLibrary() const { return m_shaderLibrary; }
    
    // Pipeline state objects
    MetalRenderPipelineStateRef GetTrianglePipeline() const { return m_trianglePipeline; }
    MetalRenderPipelineStateRef GetLinePipeline() const { return m_linePipeline; }
    MetalDepthStencilStateRef GetDepthState() const { return m_depthState; }
    
private:
    bool CreatePipelineStates();
    bool LoadShaders();
    
    MetalDeviceRef m_device;
    MetalCommandQueueRef m_commandQueue;
    MetalLibraryRef m_shaderLibrary;
    MetalRenderPipelineStateRef m_trianglePipeline;
    MetalRenderPipelineStateRef m_linePipeline;
    MetalDepthStencilStateRef m_depthState;
};

// Metal-based backend implementation
class MetalBackend : public Backend {
public:
    MetalBackend();
    ~MetalBackend() override;
    
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
    std::unique_ptr<MetalDevice> m_device;
    std::vector<std::shared_ptr<Texture>> m_textures;
    std::vector<std::shared_ptr<RenderTarget>> m_renderTargets;
    RenderTarget* m_currentRenderTarget = nullptr;
    BlendMode m_currentBlendMode = BlendMode::None;
    std::unique_ptr<MetalCommandBuffer> m_currentCommandBuffer;
};

} // namespace metal
} // namespace labfont

#endif // LABFONT_METAL_BACKEND_H
