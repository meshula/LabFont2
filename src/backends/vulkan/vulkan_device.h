#ifndef LABFONT_VULKAN_DEVICE_H
#define LABFONT_VULKAN_DEVICE_H

#include "vulkan_types.h"
#include <vector>
#include <string>

namespace labfont {
namespace vulkan {

class VulkanDevice {
public:
    VulkanDevice();
    ~VulkanDevice();

    bool Initialize();
    void Shutdown();

    // Device properties
    VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const;
    VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures() const;
    VkPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties() const;
    uint32_t GetMaxTextureSize() const;

    // Memory management
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // Format support
    VkFormat GetVulkanFormat(TextureFormat format) const;
    bool SupportsFormat(TextureFormat format) const;

    // Getters
    VkDevice GetDevice() const { return m_device; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    VkQueue GetGraphicsQueue() const { return m_graphicsQueue; }
    uint32_t GetGraphicsQueueFamily() const { return m_graphicsQueueFamily; }
    VkCommandPool GetCommandPool() const { return m_commandPool; }

private:
    bool CreateInstance();
    bool CreatePhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateCommandPool();

    bool CheckValidationLayerSupport();
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char*> GetRequiredExtensions();

    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = 0;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

    // Validation layers
    bool m_enableValidationLayers = true;
    std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    // Device extensions
    std::vector<const char*> m_deviceExtensions = {
#ifdef __APPLE__
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME,  // Required for viewport flipping
        VK_KHR_MAINTENANCE2_EXTENSION_NAME   // Required for input attachments
#endif
    };
};

} // namespace vulkan
} // namespace labfont

#endif // LABFONT_VULKAN_DEVICE_H
