#ifndef LABFONT_VULKAN_SHADER_COMPILER_H
#define LABFONT_VULKAN_SHADER_COMPILER_H

#include "vulkan_types.h"
#include <vector>
#include <string>

namespace labfont {
namespace vulkan {

class VulkanShaderCompiler {
public:
    static VkShaderModule CreateShaderModule(VkDevice device, const std::string& filename);
    static VkShaderModule CreateShaderModuleFromSPV(VkDevice device, const std::vector<uint32_t>& code);
    
private:
    static std::vector<uint32_t> ReadSPVFile(const std::string& filename);
};

} // namespace vulkan
} // namespace labfont

#endif // LABFONT_VULKAN_SHADER_COMPILER_H
