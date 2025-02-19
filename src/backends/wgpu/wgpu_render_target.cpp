#include "wgpu_backend.h"
#include "wgpu_device.h"
#include <cstring>

namespace labfont {

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
        .data = nullptr,
        .renderTarget = true,
        .readback = true,
        .dataSize = 0
    };
    m_colorTexture = std::make_shared<WebGPUTexture>(device, colorDesc);
    
    // Create depth texture if needed
    if (m_hasDepth) {
        TextureDesc depthDesc = {
            .width = desc.width,
            .height = desc.height,
            .format = TextureFormat::R32F,
            .data = nullptr,
            .renderTarget = true,
            .readback = false,
            .dataSize = 0
        };
        m_depthTexture = std::make_shared<WebGPUTexture>(device, depthDesc);
    }
    
    // Create render pass descriptor
    m_renderPassDesc = new WGPURenderPassDescriptor();
    m_renderPassDesc->colorAttachmentCount = 1;
    
    WGPURenderPassColorAttachment* colorAttachment = new WGPURenderPassColorAttachment();
    colorAttachment->view = m_colorTexture->GetWGPUTextureView();
    colorAttachment->resolveTarget = nullptr;
    colorAttachment->loadOp = WGPULoadOp_Clear;
    colorAttachment->storeOp = WGPUStoreOp_Store;
    colorAttachment->clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
    m_renderPassDesc->colorAttachments = colorAttachment;
    
    if (m_hasDepth) {
        WGPURenderPassDepthStencilAttachment* depthAttachment = new WGPURenderPassDepthStencilAttachment();
        depthAttachment->view = m_depthTexture->GetWGPUTextureView();
        depthAttachment->depthLoadOp = WGPULoadOp_Clear;
        depthAttachment->depthStoreOp = WGPUStoreOp_Store;
        depthAttachment->depthClearValue = 1.0f;
        depthAttachment->stencilLoadOp = WGPULoadOp_Clear;
        depthAttachment->stencilStoreOp = WGPUStoreOp_Store;
        depthAttachment->stencilClearValue = 0;
        m_renderPassDesc->depthStencilAttachment = depthAttachment;
    } else {
        m_renderPassDesc->depthStencilAttachment = nullptr;
    }
}

WebGPURenderTarget::~WebGPURenderTarget() {
    if (m_renderPassDesc) {
        delete[] m_renderPassDesc->colorAttachments;
        if (m_renderPassDesc->depthStencilAttachment) {
            delete m_renderPassDesc->depthStencilAttachment;
        }
        delete m_renderPassDesc;
    }
}

} // namespace labfont
