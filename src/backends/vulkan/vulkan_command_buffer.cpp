#include "vulkan_command_buffer.h"
#include "vulkan_backend.h"
#include <cassert>

namespace labfont {
namespace vulkan {

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device)
    : m_device(device)
    , m_commandBuffer(VK_NULL_HANDLE)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(m_device->GetDevice(), &allocInfo, &m_commandBuffer);
    assert(result == VK_SUCCESS);
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    if (m_commandBuffer != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(m_device->GetDevice(), m_device->GetCommandPool(), 1, &m_commandBuffer);
    }
}

bool VulkanCommandBuffer::Begin() {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult result = vkBeginCommandBuffer(m_commandBuffer, &beginInfo);
    return result == VK_SUCCESS;
}

bool VulkanCommandBuffer::End() {
    VkResult result = vkEndCommandBuffer(m_commandBuffer);
    return result == VK_SUCCESS;
}

bool VulkanCommandBuffer::Submit() {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    VkResult result = vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    return result == VK_SUCCESS;
}

bool VulkanCommandBuffer::BeginRenderPass(VulkanRenderTarget* target) {
    if (!target) {
        return false;
    }

    const auto& desc = target->GetRenderPassDesc();

    VkRenderPassBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = desc.renderPass;
    beginInfo.framebuffer = desc.framebuffer;
    beginInfo.renderArea.offset = {0, 0};
    beginInfo.renderArea.extent = {desc.width, desc.height};
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &desc.clearValue;

    vkCmdBeginRenderPass(m_commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    return true;
}

void VulkanCommandBuffer::EndRenderPass() {
    vkCmdEndRenderPass(m_commandBuffer);
}

void VulkanCommandBuffer::BindPipeline(VkPipeline pipeline) {
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void VulkanCommandBuffer::SetViewport(float x, float y, float width, float height) {
    VkViewport viewport = {};
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
}

void VulkanCommandBuffer::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) {
    VkRect2D scissor = {};
    scissor.offset = {x, y};
    scissor.extent = {width, height};

    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

} // namespace vulkan
} // namespace labfont
