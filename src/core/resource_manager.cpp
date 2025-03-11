#include "resource_manager.h"
#include "backend.h"
#include "error_macros.h"
#include "context_internal.h"
#include <cassert>
#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb/stb_image.h"

namespace labfont {

ResourceManagerImpl::ResourceManagerImpl(Backend* backend)
    : m_backend(backend)
{
    // Backend can be null for testing
}

ResourceManagerImpl::~ResourceManagerImpl()
{
    // Clear all resources
    std::lock_guard<std::mutex> lock(m_mutex);
    m_resources.clear();
}

lab_result ResourceManagerImpl::CreateTexture(
    const std::string& name,
    const TextureParams& params,
    std::shared_ptr<TextureResource>& out_texture)
{
    LAB_RESULT_GUARD();

    if (name.empty()) {
        return LAB_RESULT_INVALID_RESOURCE_NAME;
    }

    if (params.width == 0 || params.height == 0) {
        return LAB_RESULT_INVALID_DIMENSION;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (ResourceExists(name)) {
        return LAB_RESULT_DUPLICATE_RESOURCE_NAME;
    }

    auto texture = std::make_shared<TextureResource>(
        name,
        params.width,
        params.height,
        params.format
    );

    // TODO: Once backend interface is implemented, create the actual GPU resource
    // lab_result result = m_backend->CreateTexture(params, texture.get());
    // LAB_RETURN_IF_ERROR(result);

    m_resources[name] = texture;
    out_texture = texture;
    return LAB_RESULT_OK;
}

lab_result ResourceManagerImpl::CreateBuffer(
    const std::string& name,
    const BufferParams& params,
    std::shared_ptr<BufferResource>& out_buffer)
{
    LAB_RESULT_GUARD();

    if (name.empty()) {
        return LAB_RESULT_INVALID_RESOURCE_NAME;
    }

    if (params.size == 0) {
        return LAB_RESULT_INVALID_BUFFER_SIZE;
   }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (ResourceExists(name)) {
        return LAB_RESULT_DUPLICATE_RESOURCE_NAME;
    }

    auto buffer = std::make_shared<BufferResource>(
        name,
        params.size,
        params.dynamic
    );

    // TODO: Once backend interface is implemented, create the actual GPU resource
    // lab_result result = m_backend->CreateBuffer(params, buffer.get());
    // LAB_RETURN_IF_ERROR(result);

    m_resources[name] = buffer;
    out_buffer = buffer;
    return LAB_RESULT_OK;
}

lab_result ResourceManagerImpl::CreateRenderTarget(
    const std::string& name,
    const RenderTargetParams& params,
    std::shared_ptr<RenderTargetResource>& out_target)
{
    LAB_RESULT_GUARD();

    if (name.empty()) {
        return LAB_RESULT_INVALID_RESOURCE_NAME;
    }

    if (params.width == 0 || params.height == 0) {
        return LAB_RESULT_INVALID_DIMENSION;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (ResourceExists(name)) {
        return LAB_RESULT_DUPLICATE_RESOURCE_NAME;
    }

    auto target = std::make_shared<RenderTargetResource>(
        name,
        params.width,
        params.height,
        params.format,
        params.hasDepth
    );

    // Create the actual GPU render target
    RenderTargetDesc desc;
    desc.width = params.width;
    desc.height = params.height;
    desc.format = params.format;
    desc.hasDepth = params.hasDepth;
    
    // Create the backend render target
    std::shared_ptr<RenderTarget> backendTarget;
    if (m_backend) {
        lab_result result = m_backend->CreateRenderTarget(desc, backendTarget);
        if (result != LAB_RESULT_OK) {
            return result;
        }
        
        // Store the backend render target in the resource
        target->SetBackendTarget(backendTarget);
    }

    // Store the resource
    m_resources[name] = target;
    out_target = target;
    return LAB_RESULT_OK;
}

void ResourceManagerImpl::DestroyResource(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    RemoveResource(name);
}

std::shared_ptr<Resource> ResourceManagerImpl::GetResource(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_resources.find(name);
    return (it != m_resources.end()) ? it->second : nullptr;
}

bool ResourceManagerImpl::ResourceExists(const std::string& name)
{
    // Caller must hold mutex
    return m_resources.find(name) != m_resources.end();
}

void ResourceManagerImpl::RemoveResource(const std::string& name)
{
    // Caller must hold mutex
    auto it = m_resources.find(name);
    if (it != m_resources.end()) {
        // TODO: Once backend interface is implemented
        // m_backend->DestroyResource(it->second.get());
        m_resources.erase(it);
    }
}

} // namespace labfont

// C API implementations for resource management
extern "C" {

lab_result lab_create_render_target(lab_context ctx, const lab_render_target_desc* desc, lab_render_target* out_target) {
    if (!ctx) {
        return LAB_RESULT_INVALID_CONTEXT;
    }
    if (!out_target) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    if (!desc) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto resourceManager = context->GetResourceManager();
    
    // Convert C API render target desc to internal format
    labfont::RenderTargetParams params;
    params.width = desc->width;
    params.height = desc->height;
    params.format = desc->format;
    params.hasDepth = desc->hasDepth;
    
    // Generate a unique name for the render target
    std::string name = "render_target_" + std::to_string(reinterpret_cast<uintptr_t>(desc));
    
    std::shared_ptr<labfont::RenderTargetResource> target;
    auto result = resourceManager->CreateRenderTarget(name, params, target);
    
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
    auto resourceManager = context->GetResourceManager();
    
    auto targetResource = reinterpret_cast<labfont::RenderTargetResource*>(target);
    resourceManager->DestroyResource(targetResource->GetName());
}

lab_result lab_create_texture(lab_context ctx, const lab_texture_desc* desc, lab_texture* out_texture) {
    if (!out_texture) {
        return LAB_RESULT_INVALID_TEXTURE;
    }
    if (!desc) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    // If context is NULL, we just create a texture descriptor without a backend
    if (!ctx) {
        // Create a simple texture descriptor that can be used later
        auto texture = new labfont::TextureResource(
            "texture_" + std::to_string(reinterpret_cast<uintptr_t>(desc)),
            desc->width,
            desc->height,
            desc->format
        );
        
        *out_texture = reinterpret_cast<lab_texture>(texture);
        return LAB_RESULT_OK;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto resourceManager = context->GetResourceManager();
    
    // Convert C API texture desc to internal format
    labfont::TextureParams params;
    params.width = desc->width;
    params.height = desc->height;
    params.format = desc->format;
    params.data = desc->initial_data;
    
    // Generate a unique name for the texture
    std::string name = "texture_" + std::to_string(reinterpret_cast<uintptr_t>(desc));
    
    std::shared_ptr<labfont::TextureResource> texture;
    auto result = resourceManager->CreateTexture(name, params, texture);
    
    if (result == LAB_RESULT_OK) {
        *out_texture = reinterpret_cast<lab_texture>(texture.get());
    }
    
    return result;
}

void lab_destroy_texture(lab_context ctx, lab_texture texture) {
    if (!ctx || !texture) {
        return;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto resourceManager = context->GetResourceManager();
    
    auto textureResource = reinterpret_cast<labfont::TextureResource*>(texture);
    resourceManager->DestroyResource(textureResource->GetName());
}

lab_result lab_create_buffer(lab_context ctx, const lab_buffer_desc* desc, lab_buffer* out_buffer) {
    if (!ctx) {
        return LAB_RESULT_INVALID_CONTEXT;
    }
    if (!out_buffer) {
        return LAB_RESULT_INVALID_TEXTURE;
    }
    if (!desc) {
        return LAB_RESULT_INVALID_BUFFER;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto resourceManager = context->GetResourceManager();
    
    // Convert C API buffer desc to internal format
    labfont::BufferParams params;
    params.size = desc->size;
    params.dynamic = desc->dynamic;
    params.data = desc->initial_data;
    
    // Generate a unique name for the buffer
    std::string name = "buffer_" + std::to_string(reinterpret_cast<uintptr_t>(desc));
    
    std::shared_ptr<labfont::BufferResource> buffer;
    auto result = resourceManager->CreateBuffer(name, params, buffer);
    
    if (result == LAB_RESULT_OK) {
        *out_buffer = reinterpret_cast<lab_buffer>(buffer.get());
    }
    
    return result;
}

void lab_destroy_buffer(lab_context ctx, lab_buffer buffer) {
    if (!ctx || !buffer) {
        return;
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto resourceManager = context->GetResourceManager();
    
    auto bufferResource = reinterpret_cast<labfont::BufferResource*>(buffer);
    resourceManager->DestroyResource(bufferResource->GetName());
}

lab_result lab_load_texture(lab_context ctx, const char* path, lab_texture* out_texture) {
    if (!path || !out_texture) {
        return LAB_RESULT_INVALID_PARAMETER;
    }
    
    // Use stb_image to load the texture
    int width, height, channels;
    unsigned char* data = stbi_load(path, &width, &height, &channels, 4); // Force RGBA
    
    if (!data) {
        return LAB_RESULT_TEXTURE_CREATION_FAILED;
    }
    
    // Create texture descriptor
    lab_texture_desc desc = {
        .width = (unsigned int)width,
        .height = (unsigned int)height,
        .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
        .initial_data = data
    };
    
    // Create the texture
    lab_result result = lab_create_texture(ctx, &desc, out_texture);
    
    // Free the image data
    stbi_image_free(data);
    
    return result;
}

} // extern "C"
