#ifndef LABFONT_VULKAN_TYPES_H
#define LABFONT_VULKAN_TYPES_H

#include "core/backend_types.h"
#include "core/internal_types.h"
#include <vulkan/vulkan.h>

// Include portability subset extension
#ifdef __APPLE__
#include <vulkan/vulkan_beta.h>
#endif

namespace labfont {
namespace vulkan {

// Render pass description
struct RenderPassDesc {
    VkRenderPass renderPass;
    VkFramebuffer framebuffer;
    VkClearValue clearValue;
    uint32_t width;
    uint32_t height;
};

} // namespace vulkan
} // namespace labfont

#endif // LABFONT_VULKAN_TYPES_H
