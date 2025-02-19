#include "wgpu_backend.h"
#include "wgpu_device.h"
#include <cstring>

namespace labfont {

WebGPUTexture::WebGPUTexture(const WebGPUDevice* device, const TextureDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_renderTarget(desc.renderTarget)
    , m_readback(desc.readback)
    , m_texture(nullptr)
    , m_textureView(nullptr)
{
    WGPUTextureDescriptor textureDesc = GetWGPUTextureDescriptor(desc);
    m_texture = wgpuDeviceCreateTexture(device->GetDevice(), &textureDesc);
    
    if (m_texture) {
        WGPUTextureViewDescriptor viewDesc = {};
        viewDesc.format = textureDesc.format;
        viewDesc.dimension = WGPUTextureViewDimension_2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        viewDesc.aspect = WGPUTextureAspect_All;
        
        m_textureView = wgpuTextureCreateView(m_texture, &viewDesc);
    }
}

WebGPUTexture::~WebGPUTexture() {
    if (m_textureView) {
        wgpuTextureViewRelease(m_textureView);
    }
    if (m_texture) {
        wgpuTextureRelease(m_texture);
    }
}

} // namespace labfont
