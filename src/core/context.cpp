#include "context_internal.h"
#include <stdexcept>
#include <cassert>

namespace labfont {

lab_result Context::Create(lab_backend_type type, const lab_context_desc* desc, Context** out_context) {
    if (!desc || !out_context) {
        return {LAB_ERROR_INVALID_PARAMETER, "Invalid parameters passed to Context::Create"};
    }

    std::unique_ptr<Context> context(new (std::nothrow) Context());
    if (!context) {
        return {LAB_ERROR_OUT_OF_MEMORY, "Failed to allocate Context"};
    }

    lab_result result = context->Initialize(type, desc);
    if (result.error != LAB_ERROR_NONE) {
        return result;
    }

    *out_context = context.release();
    return {LAB_ERROR_NONE, nullptr};
}

Context::~Context() {
    // Ensure we're not in the middle of a frame
    if (m_inTextMode) {
        EndText();
    }
    if (m_inDrawMode) {
        EndDraw();
    }

    // Members will be automatically cleaned up by unique_ptr
}

lab_result Context::Initialize(lab_backend_type type, const lab_context_desc* desc) {
    m_width = desc->width;
    m_height = desc->height;
    m_inTextMode = false;
    m_inDrawMode = false;

    // Backend initialization will be implemented later
    // m_backend = CreateBackend(type);
    // if (!m_backend) {
    //     return {LAB_ERROR_BACKEND_ERROR, "Failed to create backend"};
    // }

    // Font manager initialization will be implemented later
    // m_fontManager = std::make_unique<FontManager>();
    // if (!m_fontManager) {
    //     return {LAB_ERROR_OUT_OF_MEMORY, "Failed to create font manager"};
    // }

    // Draw state initialization will be implemented later
    // m_drawState = std::make_unique<DrawState>();
    // if (!m_drawState) {
    //     return {LAB_ERROR_OUT_OF_MEMORY, "Failed to create draw state"};
    // }

    // Resource manager initialization
    m_resourceManager = std::make_unique<ResourceManagerImpl>(m_backend.get());
    if (!m_resourceManager) {
        return {LAB_ERROR_OUT_OF_MEMORY, "Failed to create resource manager"};
    }

    return {LAB_ERROR_NONE, nullptr};
}

lab_result Context::Resize(unsigned int width, unsigned int height) {
    if (width == 0 || height == 0) {
        return {LAB_ERROR_INVALID_PARAMETER, "Invalid dimensions"};
    }

    // Backend resize will be implemented later
    // if (!m_backend->Resize(width, height)) {
    //     return {LAB_ERROR_BACKEND_ERROR, "Backend resize failed"};
    // }

    m_width = width;
    m_height = height;
    return {LAB_ERROR_NONE, nullptr};
}

void Context::BeginFrame() {
    assert(!m_inTextMode && !m_inDrawMode && "BeginFrame called while in text or draw mode");
    // m_backend->BeginFrame();
}

void Context::EndFrame() {
    assert(!m_inTextMode && !m_inDrawMode && "EndFrame called while in text or draw mode");
    // m_backend->EndFrame();
}

void Context::Clear(lab_color color) {
    assert(!m_inTextMode && !m_inDrawMode && "Clear called while in text or draw mode");
    // m_backend->Clear(color);
}

void Context::BeginText() {
    assert(!m_inTextMode && !m_inDrawMode && "BeginText called while already in text or draw mode");
    m_inTextMode = true;
}

void Context::EndText() {
    assert(m_inTextMode && "EndText called without matching BeginText");
    m_inTextMode = false;
}

void Context::BeginDraw() {
    assert(!m_inTextMode && !m_inDrawMode && "BeginDraw called while already in text or draw mode");
    m_inDrawMode = true;
}

void Context::EndDraw() {
    assert(m_inDrawMode && "EndDraw called without matching BeginDraw");
    m_inDrawMode = false;
}

void Context::SetViewport(float x, float y, float width, float height) {
    // m_backend->SetViewport(x, y, width, height);
}

} // namespace labfont

// C interface implementations
extern "C" {

lab_result lab_create_context(lab_backend_type type, const lab_context_desc* desc, lab_context* out_context) {
    if (!out_context) {
        return {LAB_ERROR_INVALID_PARAMETER, "out_context is null"};
    }

    labfont::Context* context = nullptr;
    lab_result result = labfont::Context::Create(type, desc, &context);
    if (result.error == LAB_ERROR_NONE) {
        *out_context = labfont::GetContextHandle(context);
    }
    return result;
}

void lab_destroy_context(lab_context ctx) {
    if (ctx) {
        delete labfont::GetContextImpl(ctx);
    }
}

lab_result lab_resize_context(lab_context ctx, unsigned int width, unsigned int height) {
    if (!ctx) {
        return {LAB_ERROR_INVALID_PARAMETER, "Context is null"};
    }
    return labfont::GetContextImpl(ctx)->Resize(width, height);
}

void lab_begin_frame(lab_context ctx) {
    if (ctx) {
        labfont::GetContextImpl(ctx)->BeginFrame();
    }
}

void lab_end_frame(lab_context ctx) {
    if (ctx) {
        labfont::GetContextImpl(ctx)->EndFrame();
    }
}

void lab_clear(lab_context ctx, lab_color color) {
    if (ctx) {
        labfont::GetContextImpl(ctx)->Clear(color);
    }
}

void lab_set_viewport(lab_context ctx, float x, float y, float width, float height) {
    if (ctx) {
        labfont::GetContextImpl(ctx)->SetViewport(x, y, width, height);
    }
}

// Resource Management C interface
lab_result lab_create_texture(lab_context ctx, const char* name, const lab_texture_desc* desc, lab_texture* out_texture) {
    if (!ctx || !name || !desc || !out_texture) {
        return {LAB_ERROR_INVALID_PARAMETER, "Invalid parameters"};
    }

    auto context = labfont::GetContextImpl(ctx);
    labfont::TextureParams params = {
        .width = desc->width,
        .height = desc->height,
        .format = desc->format,
        .initial_data = desc->initial_data
    };

    std::shared_ptr<labfont::TextureResource> texture;
    lab_result result = context->GetResourceManager()->CreateTexture(name, params, texture);
    if (result.error == LAB_ERROR_NONE) {
        *out_texture = reinterpret_cast<lab_texture>(texture.get());
    }
    return result;
}

void lab_destroy_texture(lab_context ctx, lab_texture texture) {
    if (!ctx || !texture) return;
    auto context = labfont::GetContextImpl(ctx);
    auto resource = reinterpret_cast<labfont::TextureResource*>(texture);
    context->GetResourceManager()->DestroyResource(resource->GetName());
}

lab_texture lab_get_texture(lab_context ctx, const char* name) {
    if (!ctx || !name) return nullptr;
    auto context = labfont::GetContextImpl(ctx);
    auto resource = context->GetResourceManager()->GetResource(name);
    if (resource && resource->GetType() == labfont::ResourceType::Texture) {
        return reinterpret_cast<lab_texture>(resource.get());
    }
    return nullptr;
}

lab_result lab_create_buffer(lab_context ctx, const char* name, const lab_buffer_desc* desc, lab_buffer* out_buffer) {
    if (!ctx || !name || !desc || !out_buffer) {
        return {LAB_ERROR_INVALID_PARAMETER, "Invalid parameters"};
    }

    auto context = labfont::GetContextImpl(ctx);
    labfont::BufferParams params = {
        .size = desc->size,
        .dynamic = desc->dynamic,
        .initial_data = desc->initial_data
    };

    std::shared_ptr<labfont::BufferResource> buffer;
    lab_result result = context->GetResourceManager()->CreateBuffer(name, params, buffer);
    if (result.error == LAB_ERROR_NONE) {
        *out_buffer = reinterpret_cast<lab_buffer>(buffer.get());
    }
    return result;
}

void lab_destroy_buffer(lab_context ctx, lab_buffer buffer) {
    if (!ctx || !buffer) return;
    auto context = labfont::GetContextImpl(ctx);
    auto resource = reinterpret_cast<labfont::BufferResource*>(buffer);
    context->GetResourceManager()->DestroyResource(resource->GetName());
}

lab_buffer lab_get_buffer(lab_context ctx, const char* name) {
    if (!ctx || !name) return nullptr;
    auto context = labfont::GetContextImpl(ctx);
    auto resource = context->GetResourceManager()->GetResource(name);
    if (resource && resource->GetType() == labfont::ResourceType::Buffer) {
        return reinterpret_cast<lab_buffer>(resource.get());
    }
    return nullptr;
}

} // extern "C"
