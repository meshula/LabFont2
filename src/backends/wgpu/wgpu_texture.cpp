#include "wgpu_texture.h"
#include <cstring>

namespace labfont {

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
// Helper function to convert TextureDesc to WGPUTextureDescriptor
WGPUTextureDescriptor WebGPUTexture::GetWGPUTextureDescriptor(const TextureDesc& desc) {
    WGPUTextureDescriptor textureDesc = {};
    
    // Set texture dimensions
    textureDesc.size.width = desc.width;
    textureDesc.size.height = desc.height;
    textureDesc.size.depthOrArrayLayers = 1;
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.dimension = WGPUTextureDimension_2D;
    
    // Set format based on lab_texture_format
    switch (desc.format) {
        case LAB_TEXTURE_FORMAT_R8_UNORM:
            textureDesc.format = WGPUTextureFormat_R8Unorm;
            break;
        case LAB_TEXTURE_FORMAT_RG8_UNORM:
            textureDesc.format = WGPUTextureFormat_RG8Unorm;
            break;
        case LAB_TEXTURE_FORMAT_RGBA8_UNORM:
            textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
            break;
        case LAB_TEXTURE_FORMAT_BGRA8_UNORM_SRGB:
            textureDesc.format = WGPUTextureFormat_BGRA8UnormSrgb;
            break;
        case LAB_TEXTURE_FORMAT_R16F:
            textureDesc.format = WGPUTextureFormat_R16Float;
            break;
        case LAB_TEXTURE_FORMAT_RG16F:
            textureDesc.format = WGPUTextureFormat_RG16Float;
            break;
        case LAB_TEXTURE_FORMAT_RGBA16F:
            textureDesc.format = WGPUTextureFormat_RGBA16Float;
            break;
        case LAB_TEXTURE_FORMAT_R32F:
            textureDesc.format = WGPUTextureFormat_R32Float;
            break;
        case LAB_TEXTURE_FORMAT_RG32F:
            textureDesc.format = WGPUTextureFormat_RG32Float;
            break;
        case LAB_TEXTURE_FORMAT_RGBA32F:
            textureDesc.format = WGPUTextureFormat_RGBA32Float;
            break;
        case LAB_TEXTURE_FORMAT_D32F:
            textureDesc.format = WGPUTextureFormat_Depth32Float;
            break;
        default:
            textureDesc.format = WGPUTextureFormat_RGBA8Unorm;
            break;
    }
    
    // Set usage flags
    textureDesc.usage = WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding;
    
    if (desc.renderTarget) {
        textureDesc.usage |= WGPUTextureUsage_RenderAttachment;
    }
    
    if (desc.readback) {
        textureDesc.usage |= WGPUTextureUsage_CopySrc;
    }
    
    return textureDesc;
}

// Real implementation for Emscripten builds
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
#else
// Stub implementation for non-Emscripten builds
WebGPUTexture::WebGPUTexture(const WebGPUDevice* device, const TextureDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_renderTarget(desc.renderTarget)
    , m_readback(desc.readback)
{
    // No implementation for non-Emscripten builds
}

WebGPUTexture::~WebGPUTexture() {
    // No implementation for non-Emscripten builds
}
#endif

} // namespace labfont
