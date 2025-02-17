#include "vulkan_shader_compiler.h"
#include <fstream>
#include <stdexcept>
#include <cassert>

namespace labfont {
namespace vulkan {

VkShaderModule VulkanShaderCompiler::CreateShaderModule(VkDevice device, const std::string& filename) {
    std::vector<uint32_t> code = ReadSPVFile(filename);
    return CreateShaderModuleFromSPV(device, code);
}

VkShaderModule VulkanShaderCompiler::CreateShaderModuleFromSPV(VkDevice device, const std::vector<uint32_t>& code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    assert(result == VK_SUCCESS);

    return shaderModule;
}

std::vector<uint32_t> VulkanShaderCompiler::ReadSPVFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file: " + filename);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    return buffer;
}

} // namespace vulkan
} // namespace labfont
