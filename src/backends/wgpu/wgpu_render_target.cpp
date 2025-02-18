#include "wgpu_backend.h"
#include "wgpu_types.h"

namespace labfont {
namespace wgpu {

WGPURenderTarget::WGPURenderTarget(WGPUDevice* device, const labfont::RenderTargetDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_hasDepth(desc.hasDepth)
    , m_device(device)
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
    m_colorTexture = std::make_shared<WGPUTexture>(device, colorDesc);

    // Create depth texture if needed
    if (m_hasDepth) {
        TextureDesc depthDesc = {
            .width = desc.width,
            .height = desc.height,
            .format = TextureFormat::R32F,
            .renderTarget = true,
            .readback = false,
            .data = nullptr
        };
        m_depthTexture = std::make_shared<WGPUTexture>(device, depthDesc);
    }

    // Set up render pass descriptor
    m_renderPassDesc.colorAttachment = m_colorTexture->GetWGPUTextureView();
    m_renderPassDesc.depthStencilAttachment = m_hasDepth ? m_depthTexture->GetWGPUTextureView() : nullptr;
    m_renderPassDesc.clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    m_renderPassDesc.clearDepth = 1.0f;
    m_renderPassDesc.clearStencil = 0;
    m_renderPassDesc.label = "LabFont Render Target";
}

WGPURenderTarget::~WGPURenderTarget() = default;

} // namespace wgpu
} // namespace labfont
