#ifndef LABFONT_VULKAN_COMMAND_BUFFER_H
#define LABFONT_VULKAN_COMMAND_BUFFER_H

#include "vulkan_types.h"

namespace labfont {
namespace vulkan {

class VulkanDevice;
class VulkanRenderTarget;

class VulkanCommandBuffer {
public:
    VulkanCommandBuffer(VulkanDevice* device);
    ~VulkanCommandBuffer();

    bool Begin();
    bool End();
    bool Submit();
    
    bool BeginRenderPass(VulkanRenderTarget* target);
    void EndRenderPass();
    
    void BindPipeline(VkPipeline pipeline);
    void SetViewport(float x, float y, float width, float height);
    void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
    
    VkCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

private:
    VulkanDevice* m_device;
    VkCommandBuffer m_commandBuffer;
};

} // namespace vulkan
} // namespace labfont

#endif // LABFONT_VULKAN_COMMAND_BUFFER_H
