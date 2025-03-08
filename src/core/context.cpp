#include "context_internal.h"
#include "context_impl.h"
#include "core/memory.h"
#include "core/internal_types.h"
#include <memory>
#include <vector>

// Use stb_image_write to save the image
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party/stb/stb_image_write.h"

#ifdef LABFONT_METAL_ENABLED
#include "backends/metal/metal_backend.h"
#endif

#ifdef LABFONT_WGPU_ENABLED
#include "backends/wgpu/wgpu_backend.h"
#endif

#ifdef LABFONT_VULKAN_ENABLED
#include "backends/vulkan/vulkan_backend.h"
#endif

#ifdef LABFONT_DX11_ENABLED
#include "backends/dx11/dx11_backend.h"
#endif

// Include CPU backend for tests
#include "backends/cpu/cpu_backend.h"

namespace labfont {

Context::~Context() = default;

lab_result Context::Create(lab_backend_type type, const lab_context_desc* desc, Context** out_context) {
    auto context = std::make_unique<ContextImpl>();
    auto result = context->Initialize(type, desc);
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    *out_context = context.release();
    return lab_result(LAB_RESULT_OK);
}

// Factory function to create the appropriate backend based on the backend type
std::unique_ptr<Backend> Context::CreateBackend(lab_backend_type type) {
    switch (type) {
#ifdef LABFONT_METAL_ENABLED
        case LAB_BACKEND_METAL:
            return std::make_unique<metal::MetalBackend>();
#endif
#ifdef LABFONT_WGPU_ENABLED
        case LAB_BACKEND_WGPU:
            return std::make_unique<WGPUBackend>();
#endif
#ifdef LABFONT_VULKAN_ENABLED
        case LAB_BACKEND_VULKAN:
            return std::make_unique<vulkan::VulkanBackend>();
#endif
#ifdef LABFONT_DX11_ENABLED
        case LAB_BACKEND_DX11:
            return std::make_unique<dx11::DX11Backend>();
#endif
        // Use CPU backend as a fallback for tests
        case LAB_BACKEND_CPU:
            return std::make_unique<CPUBackend>();
        default:
            return nullptr;
    }
}

lab_result Context::Initialize(lab_backend_type type, const lab_context_desc* desc) {
    m_backend = CreateBackend(type);
    if (!m_backend) {
        return LAB_RESULT_UNSUPPORTED_BACKEND;
    }
    
    // Initialize backend
    auto result = m_backend->Initialize(desc ? desc->width : 0, desc ? desc->height : 0);
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    // Initialize managers
    m_fontManager = std::make_unique<FontManager>();
    m_drawState = std::make_unique<DrawState>();
    m_resourceManager = std::make_unique<ResourceManagerImpl>(m_backend.get());
    
    m_width = desc ? desc->width : 0;
    m_height = desc ? desc->height : 0;
    m_inTextMode = false;
    m_inDrawMode = false;
    
    return lab_result(LAB_RESULT_OK);
}

void Context::BeginFrame() {
    m_backend->BeginFrame();
}

void Context::EndFrame() {
    m_backend->EndFrame();
}

} // namespace labfont

extern "C" {

lab_result lab_create_context(const lab_backend_desc* desc, lab_context* out_context) {
    if (!desc || !out_context) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    // Convert backend_desc to context_desc
    lab_context_desc context_desc = {
        .width = desc->width,
        .height = desc->height,
        .native_window = desc->native_window,
        .max_vertices = 1024,  // Default value
        .atlas_width = 1024,   // Default value
        .atlas_height = 1024   // Default value
    };
    
    labfont::Context* context = nullptr;
    auto result = labfont::Context::Create(desc->type, &context_desc, &context);
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    *out_context = labfont::GetContextHandle(context);
    return LAB_RESULT_OK;
}

void lab_destroy_context(lab_context ctx) {
    delete labfont::GetContextImpl(ctx);
}

lab_result lab_begin_frame(lab_context ctx) {
    if (!ctx) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto result = context->GetBackend()->BeginFrame();
    return result;
}

lab_result lab_end_frame(lab_context ctx) {
    if (!ctx) {
        return LAB_RESULT_INVALID_CONTEXT;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto result = context->GetBackend()->EndFrame();
    return result;
}

lab_result lab_submit_commands(lab_context ctx, const lab_draw_command* commands, uint32_t commandCount) {
    if (!ctx) {
        return LAB_RESULT_INVALID_CONTEXT;
    }

    if (!commands || commandCount == 0) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    std::vector<labfont::DrawCommand> internalCommands(commands, commands + commandCount);
    auto result = context->GetBackend()->SubmitCommands(internalCommands);
    return result;
}

// Render target creation and destruction moved to resource_manager.cpp

lab_result lab_set_render_target(lab_context ctx, lab_render_target target) {
    if (!ctx) {
        return LAB_RESULT_INVALID_CONTEXT;
    }

    auto context = labfont::GetContextImpl(ctx);
    auto targetResource = reinterpret_cast<labfont::RenderTargetResource*>(target);
    auto backendTarget = targetResource->GetBackendTarget();
    
    auto result = context->GetBackend()->SetRenderTarget(backendTarget);
    return result;
}

lab_result lab_save_render_target(lab_context ctx, lab_render_target target, const char* filename) {
    if (!ctx || !target || !filename) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto targetResource = reinterpret_cast<labfont::RenderTargetResource*>(target);
    auto backendTarget = targetResource->GetBackendTarget();
    
    // Get the color texture from the render target
    auto colorTexture = backendTarget->GetColorTexture();
    if (!colorTexture) {
        return LAB_RESULT_INVALID_RENDER_TARGET;
    }
    
    // Check if the texture supports readback
    if (!colorTexture->SupportsReadback()) {
        return LAB_RESULT_READBACK_NOT_SUPPORTED;
    }
    
    // Get texture dimensions
    uint32_t width = colorTexture->GetWidth();
    uint32_t height = colorTexture->GetHeight();
    
    // Allocate memory for the pixel data (RGBA8 format)
    size_t dataSize = width * height * 4;
    uint8_t* pixelData = (uint8_t*)lab_alloc(dataSize, LAB_MEMORY_TEMPORARY);
    if (!pixelData) {
        return LAB_RESULT_OUT_OF_MEMORY;
    }
    
    // Read back the texture data
    lab_result result = context->GetBackend()->ReadbackTexture(colorTexture, pixelData, dataSize);
    if (result != LAB_RESULT_OK) {
        lab_free(pixelData);
        return result;
    }
    
    // Determine file format based on extension
    const char* ext = strrchr(filename, '.');
    if (!ext) {
        lab_free(pixelData);
        return LAB_RESULT_INVALID_PARAMETER; // No extension
    }
    
    int success = 0;
    if (strcmp(ext, ".png") == 0) {
        success = stbi_write_png(filename, width, height, 4, pixelData, width * 4);
    } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
        success = stbi_write_jpg(filename, width, height, 4, pixelData, 90); // Quality 90
    } else if (strcmp(ext, ".bmp") == 0) {
        success = stbi_write_bmp(filename, width, height, 4, pixelData);
    } else if (strcmp(ext, ".tga") == 0) {
        success = stbi_write_tga(filename, width, height, 4, pixelData);
    } else {
        lab_free(pixelData);
        return LAB_RESULT_UNSUPPORTED_FORMAT;
    }
    
    lab_free(pixelData);
    
    if (!success) {
        return LAB_RESULT_BACKEND_ERROR;
    }
    
    return LAB_RESULT_OK;
}

lab_result lab_resize_render_target(lab_context ctx, lab_render_target target, uint32_t width, uint32_t height) {
    if (!ctx || !target) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    if (width == 0 || height == 0) {
        return LAB_RESULT_INVALID_DIMENSION;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto targetResource = reinterpret_cast<labfont::RenderTargetResource*>(target);
    auto backendTarget = targetResource->GetBackendTarget();
    
    // Get current render target properties
    auto format = backendTarget->GetFormat();
    bool hasDepth = backendTarget->HasDepth();
    
    // Create a new render target descriptor with the new dimensions
    labfont::RenderTargetDesc desc;
    desc.width = width;
    desc.height = height;
    desc.format = format;
    desc.hasDepth = hasDepth;
    
    // Create a new backend render target
    std::shared_ptr<labfont::RenderTarget> newBackendTarget;
    lab_result result = context->GetBackend()->CreateRenderTarget(desc, newBackendTarget);
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    // Update the resource with the new backend target
    targetResource->SetBackendTarget(newBackendTarget);
    
    // Set the render target to ensure it's active
    result = context->GetBackend()->SetRenderTarget(newBackendTarget.get());
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    return LAB_RESULT_OK;
}

lab_result lab_get_render_target_data(
        lab_context ctx, lab_render_target target,
        lab_render_target_desc* desc, uint8_t** out_data, size_t* out_size) {
    if (!ctx || !target || !out_data || !out_size) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto targetResource = reinterpret_cast<labfont::RenderTargetResource*>(target);
    auto backendTarget = targetResource->GetBackendTarget();
    
    // Get the color texture from the render target
    auto colorTexture = backendTarget->GetColorTexture();
    if (!colorTexture) {
        return LAB_RESULT_INVALID_RENDER_TARGET;
    }
    
    // Check if the texture supports readback
    if (!colorTexture->SupportsReadback()) {
        return LAB_RESULT_READBACK_NOT_SUPPORTED;
    }
    
    // Get texture dimensions
    uint32_t width = colorTexture->GetWidth();
    uint32_t height = colorTexture->GetHeight();
    
    // Calculate data size (RGBA8 format)
    size_t dataSize = width * height * 4;
    
    // Check if we need to allocate or reallocate memory
    if (*out_data == nullptr || *out_size < dataSize) {
        // Free existing memory if it's too small
        if (*out_data != nullptr) {
            lab_free(*out_data);
        }
        
        // Allocate new memory
        *out_data = (uint8_t*)lab_alloc(dataSize, LAB_MEMORY_GRAPHICS);
        if (!*out_data) {
            *out_size = 0;
            return LAB_RESULT_OUT_OF_MEMORY;
        }
    }
    
    // Update the size
    *out_size = dataSize;
    
    // Read back the texture data
    lab_result result = context->GetBackend()->ReadbackTexture(colorTexture, *out_data, dataSize);
    if (result != LAB_RESULT_OK) {
        return result;
    }

    // Fill the descriptor
    if (desc) {
        desc->width = width;
        desc->height = height;
        desc->format = colorTexture->GetFormat();
    }
    
    return LAB_RESULT_OK;
}

} // extern "C"
