#include "context_internal.h"
#include "context_impl.h"
#include "core/memory.h"
#include "core/internal_types.h"
#include <memory>
#include <vector>

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

lab_result lab_create_render_target(lab_context ctx, const lab_render_target_desc* desc, lab_render_target* out_target) {
    if (!ctx) {
        return LAB_RESULT_INVALID_CONTEXT;
    }

    if (!desc || !out_target) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    labfont::RenderTargetDesc internal_desc = {
        .width = desc->width,
        .height = desc->height,
        .format = static_cast<labfont::TextureFormat>(desc->format),
        .hasDepth = desc->hasDepth
    };
    
    std::shared_ptr<labfont::RenderTarget> target;
    auto result = context->GetBackend()->CreateRenderTarget(internal_desc, target);
    if (result == LAB_RESULT_OK) {
        *out_target = reinterpret_cast<lab_render_target>(target.get());
    }
    return result;
}

void lab_destroy_render_target(lab_context ctx, lab_render_target target) {
    if (!ctx || !target) {
        return;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    context->GetBackend()->DestroyRenderTarget(reinterpret_cast<labfont::RenderTarget*>(target));
}

lab_result lab_set_render_target(lab_context ctx, lab_render_target target) {
    if (!ctx) {
        return LAB_RESULT_INVALID_CONTEXT;
    }

    auto context = labfont::GetContextImpl(ctx);
    auto result = context->GetBackend()->SetRenderTarget(reinterpret_cast<labfont::RenderTarget*>(target));
    return result;
}

} // extern "C"
