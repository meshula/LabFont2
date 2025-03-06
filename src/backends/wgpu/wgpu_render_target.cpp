#include "wgpu_render_target.h"

namespace labfont {

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
// Real implementation for Emscripten builds
WebGPURenderTarget::WebGPURenderTarget(const WebGPUDevice* device, const RenderTargetDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_hasDepth(desc.hasDepth)
    , m_renderPassDesc(nullptr)
{
    // Create color texture
    TextureDesc colorDesc = {
        .width = desc.width,
        .height = desc.height,
        .format = desc.format,
        .renderTarget = true,
        .readback = true,
        .data = nullptr
    };
    m_colorTexture = std::make_shared<WebGPUTexture>(device, colorDesc);
    
    // Create depth texture if needed
    if (desc.hasDepth) {
        TextureDesc depthDesc = {
            .width = desc.width,
            .height = desc.height,
            .format = TextureFormat::D32F,
            .renderTarget = true,
            .readback = false,
            .data = nullptr
        };
        m_depthTexture = std::make_shared<WebGPUTexture>(device, depthDesc);
    }
    
    // Create render pass descriptor
    m_renderPassDesc = new WGPURenderPassDescriptor();
    if (m_renderPassDesc) {
        // Set up color attachment
        WGPURenderPassColorAttachment colorAttachment = {};
        colorAttachment.view = m_colorTexture->GetWGPUTextureView();
        colorAttachment.resolveTarget = nullptr;
        colorAttachment.loadOp = WGPULoadOp_Clear;
        colorAttachment.storeOp = WGPUStoreOp_Store;
        colorAttachment.clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
        
        m_renderPassDesc->colorAttachmentCount = 1;
        m_renderPassDesc->colorAttachments = &colorAttachment;
        
        // Set up depth attachment if needed
        if (m_hasDepth && m_depthTexture) {
            WGPURenderPassDepthStencilAttachment depthAttachment = {};
            depthAttachment.view = m_depthTexture->GetWGPUTextureView();
            depthAttachment.depthLoadOp = WGPULoadOp_Clear;
            depthAttachment.depthStoreOp = WGPUStoreOp_Store;
            depthAttachment.depthClearValue = 1.0f;
            depthAttachment.stencilLoadOp = WGPULoadOp_Clear;
            depthAttachment.stencilStoreOp = WGPUStoreOp_Store;
            depthAttachment.stencilClearValue = 0;
            
            m_renderPassDesc->depthStencilAttachment = &depthAttachment;
        } else {
            m_renderPassDesc->depthStencilAttachment = nullptr;
        }
    }
}

WebGPURenderTarget::~WebGPURenderTarget() {
    if (m_renderPassDesc) {
        delete m_renderPassDesc;
    }
}
#else
// Stub implementation for non-Emscripten builds
WebGPURenderTarget::WebGPURenderTarget(const WebGPUDevice* device, const RenderTargetDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_hasDepth(desc.hasDepth)
{
    // No implementation for non-Emscripten builds
}

WebGPURenderTarget::~WebGPURenderTarget() {
    // No implementation for non-Emscripten builds
}
#endif

} // namespace labfont
