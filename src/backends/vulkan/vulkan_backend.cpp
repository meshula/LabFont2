#include "vulkan_backend.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include <cassert>

namespace labfont {
namespace vulkan {

// VulkanTexture implementation
VulkanTexture::VulkanTexture(const labfont::TextureDesc& desc, VulkanDevice* device)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_renderTarget(desc.renderTarget)
    , m_readback(desc.readback)
    , m_image(VK_NULL_HANDLE)
    , m_memory(VK_NULL_HANDLE)
    , m_imageView(VK_NULL_HANDLE)
    , m_device(device)
{
    // Create image
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_device->GetVulkanFormat(m_format);
    imageInfo.tiling = m_readback ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    if (m_renderTarget) {
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (m_readback) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VkResult result = vkCreateImage(m_device->GetDevice(), &imageInfo, nullptr, &m_image);
    assert(result == VK_SUCCESS);

    // Allocate memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device->GetDevice(), m_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_device->FindMemoryType(
        memRequirements.memoryTypeBits,
        m_readback ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    result = vkAllocateMemory(m_device->GetDevice(), &allocInfo, nullptr, &m_memory);
    assert(result == VK_SUCCESS);

    vkBindImageMemory(m_device->GetDevice(), m_image, m_memory, 0);

    // Create image view
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    result = vkCreateImageView(m_device->GetDevice(), &viewInfo, nullptr, &m_imageView);
    assert(result == VK_SUCCESS);
}

VulkanTexture::~VulkanTexture() {
    if (m_imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device->GetDevice(), m_imageView, nullptr);
    }
    if (m_image != VK_NULL_HANDLE) {
        vkDestroyImage(m_device->GetDevice(), m_image, nullptr);
    }
    if (m_memory != VK_NULL_HANDLE) {
        vkFreeMemory(m_device->GetDevice(), m_memory, nullptr);
    }
}

// VulkanRenderTarget implementation
VulkanRenderTarget::VulkanRenderTarget(const labfont::RenderTargetDesc& desc, VulkanDevice* device)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_hasDepth(desc.hasDepth)
    , m_renderPass(VK_NULL_HANDLE)
    , m_framebuffer(VK_NULL_HANDLE)
    , m_device(device)
{
    // Create color texture
    labfont::TextureDesc colorDesc = {};
    colorDesc.width = m_width;
    colorDesc.height = m_height;
    colorDesc.format = m_format;
    colorDesc.renderTarget = true;
    colorDesc.readback = false;
    m_colorTexture = std::make_shared<VulkanTexture>(colorDesc, device);

        // Create depth texture if needed
        if (m_hasDepth) {
            labfont::TextureDesc depthDesc = {};
            depthDesc.width = m_width;
            depthDesc.height = m_height;
            depthDesc.format = TextureFormat::D32F;
            depthDesc.renderTarget = true;
            depthDesc.readback = false;
            m_depthTexture = std::make_shared<VulkanTexture>(depthDesc, device);
        }

    // Create render pass
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colorRefs;
    VkAttachmentReference depthRef = {};

    // Color attachment
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_device->GetVulkanFormat(m_format);
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachments.push_back(colorAttachment);

    VkAttachmentReference colorRef = {};
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorRefs.push_back(colorRef);

    // Depth attachment if needed
    if (m_hasDepth) {
        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = VK_FORMAT_D32_SFLOAT;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments.push_back(depthAttachment);

        depthRef.attachment = 1;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
    subpass.pColorAttachments = colorRefs.data();
    subpass.pDepthStencilAttachment = m_hasDepth ? &depthRef : nullptr;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkResult result = vkCreateRenderPass(m_device->GetDevice(), &renderPassInfo, nullptr, &m_renderPass);
    assert(result == VK_SUCCESS);

    // Create framebuffer
    std::vector<VkImageView> attachmentViews;
    attachmentViews.push_back(m_colorTexture->GetImageView());
    if (m_hasDepth) {
        attachmentViews.push_back(m_depthTexture->GetImageView());
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.width = m_width;
    framebufferInfo.height = m_height;
    framebufferInfo.layers = 1;

    result = vkCreateFramebuffer(m_device->GetDevice(), &framebufferInfo, nullptr, &m_framebuffer);
    assert(result == VK_SUCCESS);
}

VulkanRenderTarget::~VulkanRenderTarget() {
    if (m_framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(m_device->GetDevice(), m_framebuffer, nullptr);
    }
    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device->GetDevice(), m_renderPass, nullptr);
    }
}

// VulkanBackend implementation
VulkanBackend::VulkanBackend() {}

VulkanBackend::~VulkanBackend() {
    if (m_trianglePipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->GetDevice(), m_trianglePipeline, nullptr);
    }
    if (m_linePipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_device->GetDevice(), m_linePipeline, nullptr);
    }
    if (m_pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_device->GetDevice(), m_pipelineLayout, nullptr);
    }
}

lab_result VulkanBackend::Initialize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    
    m_device = std::make_unique<VulkanDevice>();
    if (!m_device->Initialize()) {
        return lab_result{ LAB_ERROR_INITIALIZATION_FAILED };
    }
    
    if (!CreatePipelines()) {
        return lab_result{ LAB_ERROR_INITIALIZATION_FAILED };
    }
    
    return lab_result{ LAB_ERROR_NONE };
}

lab_result VulkanBackend::Resize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return lab_result{ LAB_ERROR_NONE };
}

lab_result VulkanBackend::CreateTexture(const labfont::TextureDesc& desc, std::shared_ptr<labfont::Texture>& out_texture) {
    try {
        auto texture = std::make_shared<VulkanTexture>(desc, m_device.get());
        m_textures.push_back(texture);
        out_texture = texture;
        return lab_result{ LAB_ERROR_NONE };
    } catch (const std::exception&) {
        return lab_result{ LAB_ERROR_OUT_OF_MEMORY };
    }
}

lab_result VulkanBackend::UpdateTexture(labfont::Texture* texture, const void* data, size_t size) {
    auto vulkanTexture = static_cast<VulkanTexture*>(texture);
    // TODO: Implement texture update
    return lab_result{ LAB_ERROR_NONE };
}

lab_result VulkanBackend::ReadbackTexture(labfont::Texture* texture, void* data, size_t size) {
    auto vulkanTexture = static_cast<VulkanTexture*>(texture);
    if (!vulkanTexture->SupportsReadback()) {
        return lab_result{ LAB_ERROR_INVALID_OPERATION };
    }
    // TODO: Implement texture readback
    return lab_result{ LAB_ERROR_NONE };
}

lab_result VulkanBackend::CreateRenderTarget(const labfont::RenderTargetDesc& desc, std::shared_ptr<labfont::RenderTarget>& out_target) {
    try {
        auto target = std::make_shared<VulkanRenderTarget>(desc, m_device.get());
        m_renderTargets.push_back(target);
        out_target = target;
        return lab_result{ LAB_ERROR_NONE };
    } catch (const std::exception&) {
        return lab_result{ LAB_ERROR_OUT_OF_MEMORY };
    }
}

lab_result VulkanBackend::SetRenderTarget(labfont::RenderTarget* target) {
    m_currentRenderTarget = target;
    return lab_result{ LAB_ERROR_NONE };
}

lab_result VulkanBackend::BeginFrame() {
    m_currentCommandBuffer = std::make_unique<VulkanCommandBuffer>(m_device.get());
    if (!m_currentCommandBuffer->Begin()) {
        return lab_result{ LAB_ERROR_COMMAND_BUFFER };
    }
    return lab_result{ LAB_ERROR_NONE };
}

lab_result VulkanBackend::SubmitCommands(const std::vector<DrawCommand>& commands) {
    // TODO: Implement command submission
    return lab_result{ LAB_ERROR_NONE };
}

lab_result VulkanBackend::EndFrame() {
    if (!m_currentCommandBuffer->End()) {
        return lab_result{ LAB_ERROR_COMMAND_BUFFER };
    }
    m_currentCommandBuffer.reset();
    return lab_result{ LAB_ERROR_NONE };
}

void VulkanBackend::DestroyTexture(labfont::Texture* texture) {
    for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
        if (it->get() == texture) {
            m_textures.erase(it);
            break;
        }
    }
}

void VulkanBackend::DestroyRenderTarget(labfont::RenderTarget* target) {
    for (auto it = m_renderTargets.begin(); it != m_renderTargets.end(); ++it) {
        if (it->get() == target) {
            m_renderTargets.erase(it);
            break;
        }
    }
}

size_t VulkanBackend::GetTextureMemoryUsage() const {
    // TODO: Implement memory tracking
    return 0;
}

size_t VulkanBackend::GetTotalMemoryUsage() const {
    // TODO: Implement memory tracking
    return 0;
}

bool VulkanBackend::SupportsTextureFormat(TextureFormat format) const {
    return m_device->SupportsFormat(format);
}

bool VulkanBackend::SupportsBlendMode(BlendMode mode) const {
    return true; // All blend modes supported
}

uint32_t VulkanBackend::GetMaxTextureSize() const {
    return m_device->GetMaxTextureSize();
}

bool VulkanBackend::CreatePipelines() {
    // TODO: Implement pipeline creation
    return true;
}

} // namespace vulkan
} // namespace labfont
