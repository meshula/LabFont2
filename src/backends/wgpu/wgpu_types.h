#ifndef LABFONT_WGPU_TYPES_H
#define LABFONT_WGPU_TYPES_H

#ifndef __EMSCRIPTEN__
#error "WebGPU backend is only supported with Emscripten"
#endif

#include "core/backend_types.h"
#include <emscripten.h>
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu.h>

namespace labfont {
namespace wgpu {

// Forward declarations
class WGPURenderTarget;

// WebGPU type aliases
using WGPUDeviceRef = WGPUDevice;
using WGPUAdapterRef = WGPUAdapter;
using WGPUQueueRef = WGPUQueue;
using WGPUSurfaceRef = WGPUSurface;
using WGPUShaderModuleRef = WGPUShaderModule;
using WGPURenderPipelineRef = WGPURenderPipeline;
using WGPUBindGroupRef = WGPUBindGroup;
using WGPUBindGroupLayoutRef = WGPUBindGroupLayout;
using WGPUPipelineLayoutRef = WGPUPipelineLayout;
using WGPUBufferRef = WGPUBuffer;
using WGPUTextureRef = WGPUTexture;
using WGPUTextureViewRef = WGPUTextureView;
using WGPUSamplerRef = WGPUSampler;
using WGPUCommandEncoderRef = WGPUCommandEncoder;
using WGPURenderPassEncoderRef = WGPURenderPassEncoder;
using WGPUCommandBufferRef = WGPUCommandBufferImpl*;

// Vertex structure
struct WGPUVertex {
    float position[2];
    float texcoord[2];
    float color[4];
};

// WebGPU descriptor types
struct RenderPassDesc {
    WGPUTextureView colorAttachment;
    WGPUTextureView depthStencilAttachment;
    WGPUColor clearColor;
    float clearDepth;
    uint32_t clearStencil;
    const char* label;
};

// Convert RenderPassDesc to WebGPU descriptor
inline WGPURenderPassDescriptor GetWGPURenderPassDescriptor(const RenderPassDesc& desc) {
    WGPURenderPassDescriptor renderPassDesc = {};
    renderPassDesc.nextInChain = nullptr;
    
    static WGPURenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = desc.colorAttachment;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = desc.clearColor;
    
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    
    if (desc.depthStencilAttachment) {
        static WGPURenderPassDepthStencilAttachment depthAttachment = {};
        depthAttachment.view = desc.depthStencilAttachment;
        depthAttachment.depthLoadOp = WGPULoadOp_Clear;
        depthAttachment.depthStoreOp = WGPUStoreOp_Store;
        depthAttachment.depthClearValue = desc.clearDepth;
        depthAttachment.stencilLoadOp = WGPULoadOp_Clear;
        depthAttachment.stencilStoreOp = WGPUStoreOp_Store;
        depthAttachment.stencilClearValue = desc.clearStencil;
        
        renderPassDesc.depthStencilAttachment = &depthAttachment;
    }
    
    return renderPassDesc;
}

// Utility functions
inline WGPUTextureFormat GetWGPUFormat(labfont::TextureFormat format) {
    switch (format) {
        case labfont::TextureFormat::R8_UNORM:
            return WGPUTextureFormat_R8Unorm;
        case labfont::TextureFormat::RG8_UNORM:
            return WGPUTextureFormat_RG8Unorm;
        case labfont::TextureFormat::RGBA8_UNORM:
            return WGPUTextureFormat_RGBA8Unorm;
        case labfont::TextureFormat::R16F:
            return WGPUTextureFormat_R16Float;
        case labfont::TextureFormat::RG16F:
            return WGPUTextureFormat_RG16Float;
        case labfont::TextureFormat::RGBA16F:
            return WGPUTextureFormat_RGBA16Float;
        case labfont::TextureFormat::R32F:
            return WGPUTextureFormat_R32Float;
        case labfont::TextureFormat::RG32F:
            return WGPUTextureFormat_RG32Float;
        case labfont::TextureFormat::RGBA32F:
            return WGPUTextureFormat_RGBA32Float;
        default:
            return WGPUTextureFormat_Undefined;
    }
}

inline WGPUTextureDescriptor GetWGPUTextureDescriptor(const labfont::TextureDesc& desc) {
    WGPUTextureDescriptor textureDesc = {};
    textureDesc.nextInChain = nullptr;
    textureDesc.size = {desc.width, desc.height, 1};
    textureDesc.format = GetWGPUFormat(desc.format);
    textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    
    if (desc.renderTarget) {
        textureDesc.usage |= WGPUTextureUsage_RenderAttachment;
    }
    if (desc.readback) {
        textureDesc.usage |= WGPUTextureUsage_CopySrc;
    }
    
    textureDesc.mipLevelCount = 1;
    textureDesc.sampleCount = 1;
    textureDesc.dimension = WGPUTextureDimension_2D;
    textureDesc.viewFormats = nullptr;
    textureDesc.viewFormatCount = 0;
    
    return textureDesc;
}

inline WGPUBlendState GetWGPUBlendState(labfont::BlendMode mode) {
    WGPUBlendState state = {};
    
    switch (mode) {
        case labfont::BlendMode::None:
            state.color.operation = WGPUBlendOperation_Add;
            state.color.srcFactor = WGPUBlendFactor_One;
            state.color.dstFactor = WGPUBlendFactor_Zero;
            state.alpha.operation = WGPUBlendOperation_Add;
            state.alpha.srcFactor = WGPUBlendFactor_One;
            state.alpha.dstFactor = WGPUBlendFactor_Zero;
            break;
            
        case labfont::BlendMode::Alpha:
            state.color.operation = WGPUBlendOperation_Add;
            state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
            state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
            state.alpha.operation = WGPUBlendOperation_Add;
            state.alpha.srcFactor = WGPUBlendFactor_One;
            state.alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
            break;
            
        case labfont::BlendMode::Additive:
            state.color.operation = WGPUBlendOperation_Add;
            state.color.srcFactor = WGPUBlendFactor_One;
            state.color.dstFactor = WGPUBlendFactor_One;
            state.alpha.operation = WGPUBlendOperation_Add;
            state.alpha.srcFactor = WGPUBlendFactor_One;
            state.alpha.dstFactor = WGPUBlendFactor_One;
            break;
            
        case labfont::BlendMode::Multiply:
            state.color.operation = WGPUBlendOperation_Add;
            state.color.srcFactor = WGPUBlendFactor_Dst;
            state.color.dstFactor = WGPUBlendFactor_Zero;
            state.alpha.operation = WGPUBlendOperation_Add;
            state.alpha.srcFactor = WGPUBlendFactor_DstAlpha;
            state.alpha.dstFactor = WGPUBlendFactor_Zero;
            break;
            
        case labfont::BlendMode::Screen:
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
