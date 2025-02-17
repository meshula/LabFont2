#ifndef LABFONT_VULKAN_BACKEND_H
#define LABFONT_VULKAN_BACKEND_H

#include "core/backend.h"
#include "vulkan_types.h"
#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include <memory>
#include <vector>

namespace labfont {
namespace vulkan {

// Vulkan-based texture implementation
class VulkanTexture final : public labfont::Texture {
public:
    VulkanTexture(const labfont::TextureDesc& desc, VulkanDevice* device);
    ~VulkanTexture() override;
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool IsRenderTarget() const override { return m_renderTarget; }
    bool SupportsReadback() const override { return m_readback; }
    
    // Vulkan-specific methods
    VkImage GetImage() const { return m_image; }
    VkImageView GetImageView() const { return m_imageView; }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_renderTarget;
    bool m_readback;
    VkImage m_image;
    VkDeviceMemory m_memory;
    VkImageView m_imageView;
    VulkanDevice* m_device;
};

// Vulkan-based render target implementation
class VulkanRenderTarget final : public labfont::RenderTarget {
public:
    VulkanRenderTarget(const labfont::RenderTargetDesc& desc, VulkanDevice* device);
    ~VulkanRenderTarget() override;
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool HasDepth() const override { return m_hasDepth; }
    Texture* GetColorTexture() override { return m_colorTexture.get(); }
    Texture* GetDepthTexture() override { return m_depthTexture.get(); }
    
    // Vulkan-specific methods
    VkRenderPass GetRenderPass() const { return m_renderPass; }
    VkFramebuffer GetFramebuffer() const { return m_framebuffer; }
    RenderPassDesc GetRenderPassDesc() const {
        RenderPassDesc desc;
        desc.renderPass = m_renderPass;
        desc.framebuffer = m_framebuffer;
        desc.width = m_width;
        desc.height = m_height;
        desc.clearValue = {};  // Default clear value
        desc.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};  // Black with alpha 1
        return desc;
    }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_hasDepth;
    std::shared_ptr<VulkanTexture> m_colorTexture;
    std::shared_ptr<VulkanTexture> m_depthTexture;
    VkRenderPass m_renderPass;
    VkFramebuffer m_framebuffer;
    VulkanDevice* m_device;
};

// Vulkan-based backend implementation
class VulkanBackend final : public labfont::Backend {
public:
    VulkanBackend();
    ~VulkanBackend() override;
    
    lab_result Initialize(uint32_t width, uint32_t height) override;
    lab_result Resize(uint32_t width, uint32_t height) override;
    
    lab_result CreateTexture(const labfont::TextureDesc& desc, std::shared_ptr<labfont::Texture>& out_texture) override;
    lab_result UpdateTexture(labfont::Texture* texture, const void* data, size_t size) override;
    lab_result ReadbackTexture(labfont::Texture* texture, void* data, size_t size) override;
    
    lab_result CreateRenderTarget(const labfont::RenderTargetDesc& desc, std::shared_ptr<labfont::RenderTarget>& out_target) override;
    lab_result SetRenderTarget(labfont::RenderTarget* target) override;
    
    lab_result BeginFrame() override;
    lab_result SubmitCommands(const std::vector<DrawCommand>& commands) override;
    lab_result EndFrame() override;
    
    void DestroyTexture(labfont::Texture* texture) override;
    void DestroyRenderTarget(labfont::RenderTarget* target) override;
    
    size_t GetTextureMemoryUsage() const override;
    size_t GetTotalMemoryUsage() const override;
    
    bool SupportsTextureFormat(TextureFormat format) const override;
    bool SupportsBlendMode(BlendMode mode) const override;
    uint32_t GetMaxTextureSize() const override;
    
private:
    bool CreatePipelines();
    
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    
    std::unique_ptr<VulkanDevice> m_device;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_trianglePipeline = VK_NULL_HANDLE;
    VkPipeline m_linePipeline = VK_NULL_HANDLE;
    
    std::vector<std::shared_ptr<labfont::Texture>> m_textures;
    std::vector<std::shared_ptr<labfont::RenderTarget>> m_renderTargets;
    labfont::RenderTarget* m_currentRenderTarget = nullptr;
    BlendMode m_currentBlendMode = BlendMode::None;
    std::unique_ptr<VulkanCommandBuffer> m_currentCommandBuffer;
};

} // namespace vulkan
} // namespace labfont

#endif // LABFONT_VULKAN_BACKEND_H
