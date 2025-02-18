#include "wgpu_backend.h"
#include "wgpu_types.h"

namespace labfont {
namespace wgpu {

WGPUTexture::WGPUTexture(WGPUDevice* device, const TextureDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_renderTarget(desc.renderTarget)
    , m_readback(desc.readback)
    , m_texture(nullptr)
    , m_textureView(nullptr)
    , m_device(device)
{
    WGPUTextureDescriptor textureDesc = {};
    textureDesc.size = {desc.width, desc.height, 1};
    textureDesc.format = GetWGPUFormat(desc.format);
    textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    
    if (desc.renderTarget) {
        textureDesc.usage |= WGPUTextureUsage_RenderAttachment;
    }
    if (desc.readback) {
        textureDesc.usage |= WGPUTextureUsage_CopySrc;
    }
    
    m_texture = wgpuDeviceCreateTexture(device->GetWGPUDevice(), &textureDesc);
    if (m_texture) {
        WGPUTextureViewDescriptor viewDesc = {};
        viewDesc.format = textureDesc.format;
        viewDesc.dimension = WGPUTextureViewDimension_2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        
        m_textureView = wgpuTextureCreateView(m_texture, &viewDesc);
    }
}

WGPUTexture::~WGPUTexture() {
    if (m_textureView) {
        wgpuTextureViewRelease(m_textureView);
    }
    if (m_texture) {
        wgpuTextureDestroy(m_texture);
        wgpuTextureRelease(m_texture);
    }
}

} // namespace wgpu
} // namespace labfont
