#ifndef LABFONT_WGPU_TYPES_H
#define LABFONT_WGPU_TYPES_H

#include "core/internal_types.h"
#include <webgpu/webgpu_cpp.h>

namespace labfont {

inline WGPUTextureFormat GetWGPUTextureFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8_UNORM:
            return WGPUTextureFormat_R8Unorm;
        case TextureFormat::RG8_UNORM:
            return WGPUTextureFormat_RG8Unorm;
        case TextureFormat::RGBA8_UNORM:
            return WGPUTextureFormat_RGBA8Unorm;
        case TextureFormat::R16F:
            return WGPUTextureFormat_R16Float;
        case TextureFormat::RG16F:
            return WGPUTextureFormat_RG16Float;
        case TextureFormat::RGBA16F:
            return WGPUTextureFormat_RGBA16Float;
        case TextureFormat::R32F:
            return WGPUTextureFormat_R32Float;
        case TextureFormat::RG32F:
            return WGPUTextureFormat_RG32Float;
        case TextureFormat::RGBA32F:
            return WGPUTextureFormat_RGBA32Float;
        case TextureFormat::D32F:
            return WGPUTextureFormat_Depth32Float;
        default:
            return WGPUTextureFormat_Undefined;
    }
}

inline WGPUTextureUsage GetWGPUTextureUsage(const TextureDesc& desc) {
    WGPUTextureUsageFlags usage = WGPUTextureUsage_TextureBinding;
    
    if (desc.renderTarget) {
        usage = usage | WGPUTextureUsage_RenderAttachment;
    }
    
    if (desc.readback) {
        usage = usage | WGPUTextureUsage_CopySrc;
    }
    
    if (desc.data) {
        usage = usage | WGPUTextureUsage_CopyDst;
    }
    
    return static_cast<WGPUTextureUsage>(usage);
}

inline WGPUTextureDescriptor GetWGPUTextureDescriptor(const TextureDesc& desc) {
    WGPUTextureDescriptor wgpuDesc = {};
    wgpuDesc.usage = GetWGPUTextureUsage(desc);
    wgpuDesc.dimension = WGPUTextureDimension_2D;
    wgpuDesc.size = {
        .width = desc.width,
        .height = desc.height,
        .depthOrArrayLayers = 1
    };
    wgpuDesc.format = GetWGPUTextureFormat(desc.format);
    wgpuDesc.mipLevelCount = 1;
    wgpuDesc.sampleCount = 1;
    return wgpuDesc;
}

} // namespace labfont

#endif // LABFONT_WGPU_TYPES_H
