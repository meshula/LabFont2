#ifndef LABFONT_WGPU_TEXTURE_H
#define LABFONT_WGPU_TEXTURE_H

#include "core/backend.h"
#include "wgpu_device.h"

namespace labfont {

// WebGPU-specific texture implementation
class WebGPUTexture : public Texture {
public:
    WebGPUTexture(const WebGPUDevice* device, const TextureDesc& desc);
    ~WebGPUTexture() override;
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    lab_texture_format GetFormat() const override { return m_format; }
    bool IsRenderTarget() const override { return m_renderTarget; }
    bool SupportsReadback() const override { return m_readback; }
    
    // WebGPU-specific methods
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    WGPUTexture GetWGPUTexture() const { return m_texture; }
    WGPUTextureView GetWGPUTextureView() const { return m_textureView; }
#endif

private:
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    WGPUTextureDescriptor GetWGPUTextureDescriptor(const TextureDesc& desc);
#endif

    uint32_t m_width;
    uint32_t m_height;
    lab_texture_format m_format;
    bool m_renderTarget;
    bool m_readback;
    
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    WGPUTexture m_texture;
    WGPUTextureView m_textureView;
#else
    void* m_texture = nullptr;
    void* m_textureView = nullptr;
#endif
};

} // namespace labfont

#endif // LABFONT_WGPU_TEXTURE_H
