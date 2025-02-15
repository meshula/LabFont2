#ifndef LABFONT_WGPU_TYPES_H
#define LABFONT_WGPU_TYPES_H

#include <webgpu/webgpu.h>

namespace labfont {
namespace wgpu {

// Device and adapter references
using WGPUDeviceRef = WGPUDevice;
using WGPUAdapterRef = WGPUAdapter;
using WGPUQueueRef = WGPUQueue;
using WGPUSurfaceRef = WGPUSurface;

// Pipeline and binding references
using WGPUShaderModuleRef = WGPUShaderModule;
using WGPURenderPipelineRef = WGPURenderPipeline;
using WGPUBindGroupRef = WGPUBindGroup;
using WGPUBindGroupLayoutRef = WGPUBindGroupLayout;
using WGPUPipelineLayoutRef = WGPUPipelineLayout;

// Buffer and texture references
using WGPUBufferRef = WGPUBuffer;
using WGPUTextureRef = WGPUTexture;
using WGPUTextureViewRef = WGPUTextureView;
using WGPUSamplerRef = WGPUSampler;

// Command references
using WGPUCommandEncoderRef = WGPUCommandEncoder;
using WGPURenderPassEncoderRef = WGPURenderPassEncoder;
using WGPUCommandBufferRef = WGPUCommandBuffer;

// Descriptor types
struct TextureDesc {
    uint32_t width;
    uint32_t height;
    WGPUTextureFormat format;
    WGPUTextureUsage usage;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
    const char* label;
};

struct BufferDesc {
    uint64_t size;
    WGPUBufferUsage usage;
    bool mappedAtCreation;
    const char* label;
};

struct RenderPassDesc {
    WGPUTextureView colorAttachment;
    WGPUTextureView depthStencilAttachment;
    WGPUColor clearColor;
    float clearDepth;
    uint32_t clearStencil;
    const char* label;
};

// Utility functions
inline WGPUTextureFormat GetWGPUFormat(TextureFormat format) {
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
        default:
            return WGPUTextureFormat_Undefined;
    }
}

inline WGPUBlendState GetWGPUBlendState(BlendMode mode) {
    WGPUBlendState state = {};
    
    switch (mode) {
        case BlendMode::None:
            state.color.operation = WGPUBlendOperation_Add;
            state.color.srcFactor = WGPUBlendFactor_One;
            state.color.dstFactor = WGPUBlendFactor_Zero;
            state.alpha.operation = WGPUBlendOperation_Add;
            state.alpha.srcFactor = WGPUBlendFactor_One;
            state.alpha.dstFactor = WGPUBlendFactor_Zero;
            break;
            
        case BlendMode::Alpha:
            state.color.operation = WGPUBlendOperation_Add;
            state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
            state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
            state.alpha.operation = WGPUBlendOperation_Add;
            state.alpha.srcFactor = WGPUBlendFactor_One;
            state.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
            break;
            
        case BlendMode::Additive:
            state.color.operation = WGPUBlendOperation_Add;
            state.color.srcFactor = WGPUBlendFactor_One;
            state.color.dstFactor = WGPUBlendFactor_One;
            state.alpha.operation = WGPUBlendOperation_Add;
            state.alpha.srcFactor = WGPUBlendFactor_One;
            state.alpha.dstFactor = WGPUBlendFactor_One;
            break;
            
        case BlendMode::Multiply:
            state.color.operation = WGPUBlendOperation_Add;
            state.color.srcFactor = WGPUBlendFactor_Dst;
            state.color.dstFactor = WGPUBlendFactor_Zero;
            state.alpha.operation = WGPUBlendOperation_Add;
            state.alpha.srcFactor = WGPUBlendFactor_DstAlpha;
            state.alpha.dstFactor = WGPUBlendFactor_Zero;
            break;
            
        case BlendMode::Screen:
            state.color.operation = WGPUBlendOperation_Add;
            state.color.srcFactor = WGPUBlendFactor_One;
            state.color.dstFactor = WGPUBlendFactor_OneMinusSrc;
            state.alpha.operation = WGPUBlendOperation_Add;
            state.alpha.srcFactor = WGPUBlendFactor_One;
            state.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
            break;
    }
    
    return state;
}

} // namespace wgpu
} // namespace labfont

#endif // LABFONT_WGPU_TYPES_H
