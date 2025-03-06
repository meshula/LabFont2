#include "vulkan_device.h"
#include <cassert>
#include <set>
#include <string>
#include <cstring>
#include <vector>

namespace labfont {
namespace vulkan {

VulkanDevice::VulkanDevice() {}

VulkanDevice::~VulkanDevice() {
    Shutdown();
}

bool VulkanDevice::Initialize() {
    if (!CreateInstance()) {
        return false;
    }
    
    if (!CreatePhysicalDevice()) {
        return false;
    }
    
    if (!CreateLogicalDevice()) {
        return false;
    }
    
    if (!CreateCommandPool()) {
        return false;
    }
    
    return true;
}

void VulkanDevice::Shutdown() {
    if (m_commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }
    
    if (m_device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }
    
    if (m_debugMessenger != VK_NULL_HANDLE) {
        // TODO: Destroy debug messenger
        m_debugMessenger = VK_NULL_HANDLE;
    }
    
    if (m_instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }
}

bool VulkanDevice::CreateInstance() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "LabFont";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "LabFont";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    
#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    
    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    }
    
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    return result == VK_SUCCESS;
}

bool VulkanDevice::CreatePhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        return false;
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
    
    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            m_physicalDevice = device;
            break;
        }
    }
    
    return m_physicalDevice != VK_NULL_HANDLE;
}

bool VulkanDevice::CreateLogicalDevice() {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamily;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    
    VkPhysicalDeviceFeatures deviceFeatures = {};
    
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
    
    if (m_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
    }
    
    VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
    return true;
}

bool VulkanDevice::CreateCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = m_graphicsQueueFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    VkResult result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
    return result == VK_SUCCESS;
}

uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    assert(false && "Failed to find suitable memory type");
    return 0;
}

VkFormat VulkanDevice::GetVulkanFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case TextureFormat::RG8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RGBA8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::R16F:
            return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::RG16F:
            return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::RGBA16F:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::R32F:
            return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::RG32F:
            return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::RGBA32F:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::D32F:
            return VK_FORMAT_D32_SFLOAT;
        default:
            assert(false && "Unsupported texture format");
            return VK_FORMAT_UNDEFINED;
    }
}

bool VulkanDevice::SupportsFormat(TextureFormat format) const {
    VkFormat vulkanFormat = GetVulkanFormat(format);
    if (vulkanFormat == VK_FORMAT_UNDEFINED) {
        return false;
    }
    
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(m_physicalDevice, vulkanFormat, &props);
    
    // Check if format supports being used as a color attachment
    bool supportsColor = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0;
    
    // For depth/stencil format, check depth/stencil attachment support
    bool supportsDepthStencil = true;
    if (format == TextureFormat::D32F) {
        supportsDepthStencil = (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
        return supportsDepthStencil;  // Don't check color support for depth formats
    }
    
    return supportsColor;
}

uint32_t VulkanDevice::GetMaxTextureSize() const {
    VkPhysicalDeviceProperties props = GetPhysicalDeviceProperties();
    return props.limits.maxImageDimension2D;
}

VkPhysicalDeviceProperties VulkanDevice::GetPhysicalDeviceProperties() const {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &props);
    return props;
}

VkPhysicalDeviceFeatures VulkanDevice::GetPhysicalDeviceFeatures() const {
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(m_physicalDevice, &features);
    return features;
}

VkPhysicalDeviceMemoryProperties VulkanDevice::GetPhysicalDeviceMemoryProperties() const {
    VkPhysicalDeviceMemoryProperties props;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &props);
    return props;
}

bool VulkanDevice::CheckValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    for (const char* layerName : m_validationLayers) {
        bool layerFound = false;
        
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        
        if (!layerFound) {
            return false;
        }
    }
    
    return true;
}

bool VulkanDevice::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    
    std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());
    
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    
    return requiredExtensions.empty();
}

bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    // Find queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    // Find graphics queue family
    bool foundGraphicsQueue = false;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_graphicsQueueFamily = i;
            foundGraphicsQueue = true;
            break;
        }
    }
    
    bool extensionsSupported = CheckDeviceExtensionSupport(device);
    
    // On macOS, we can't require discrete GPU since most Macs use integrated graphics
#ifdef __APPLE__
    return foundGraphicsQueue && extensionsSupported;
#else
    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader &&
           foundGraphicsQueue &&
           extensionsSupported;
#endif
}

std::vector<const char*> VulkanDevice::GetRequiredExtensions() {
    std::vector<const char*> extensions;
    
    // Add platform-specific extensions
#ifdef __APPLE__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif
    
    if (m_enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    return extensions;
}

} // namespace vulkan
} // namespace labfont
