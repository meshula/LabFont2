#ifndef LABFONT_WGPU_RENDER_TARGET_H
#define LABFONT_WGPU_RENDER_TARGET_H

#include "core/backend.h"
#include "wgpu_device.h"
#include "wgpu_texture.h"

namespace labfont {

// WebGPU-specific render target implementation
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
    
    // WebGPU-specific methods
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    WGPURenderPassDescriptor* GetRenderPassDescriptor() const { return m_renderPassDesc; }
#endif

private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_hasDepth;
    
    std::shared_ptr<WebGPUTexture> m_colorTexture;
    std::shared_ptr<WebGPUTexture> m_depthTexture;
    
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    WGPURenderPassDescriptor* m_renderPassDesc;
#else
    void* m_renderPassDesc = nullptr;
#endif
};

} // namespace labfont

#endif // LABFONT_WGPU_RENDER_TARGET_H
