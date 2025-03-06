#include "resource_manager.h"
#include "backend.h"
#include "error_macros.h"
#include "context_internal.h"
#include <cassert>

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
    LAB_ERROR_GUARD();

    if (name.empty()) {
        LAB_RETURN_ERROR(LAB_ERROR_INVALID_PARAMETER, "Resource name cannot be empty");
    }

    if (params.width == 0 || params.height == 0) {
        LAB_RETURN_ERROR(LAB_ERROR_INVALID_PARAMETER, "Invalid texture dimensions");
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (ResourceExists(name)) {
        LAB_RETURN_ERROR(LAB_ERROR_INVALID_PARAMETER, "Resource with this name already exists");
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
    return {LAB_ERROR_NONE, nullptr};
}

lab_result ResourceManagerImpl::CreateBuffer(
    const std::string& name,
    const BufferParams& params,
    std::shared_ptr<BufferResource>& out_buffer)
{
    LAB_ERROR_GUARD();

    if (name.empty()) {
        LAB_RETURN_ERROR(LAB_ERROR_INVALID_PARAMETER, "Resource name cannot be empty");
    }

    if (params.size == 0) {
        LAB_RETURN_ERROR(LAB_ERROR_INVALID_PARAMETER, "Buffer size cannot be zero");
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (ResourceExists(name)) {
        LAB_RETURN_ERROR(LAB_ERROR_INVALID_PARAMETER, "Resource with this name already exists");
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
    return {LAB_ERROR_NONE, nullptr};
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

lab_operation_result lab_create_texture(lab_context ctx, const lab_texture_desc* desc, lab_texture* out_texture) {
    if (!ctx || !desc || !out_texture) {
        return lab_operation_result{LAB_ERROR_INVALID_PARAMETER, "Invalid parameters"};
    }
    
    auto context = labfont::GetContextImpl(ctx);
    auto resourceManager = context->GetResourceManager();
    
    // Convert C API texture desc to internal format
    labfont::TextureParams params;
    params.width = desc->width;
    params.height = desc->height;
    params.format = static_cast<labfont::TextureFormat>(desc->format);
    params.data = desc->initial_data;
    
    // Generate a unique name for the texture
    std::string name = "texture_" + std::to_string(reinterpret_cast<uintptr_t>(desc));
    
    std::shared_ptr<labfont::TextureResource> texture;
    auto result = resourceManager->CreateTexture(name, params, texture);
    
    if (result.error == LAB_ERROR_NONE) {
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

lab_operation_result lab_create_buffer(lab_context ctx, const lab_buffer_desc* desc, lab_buffer* out_buffer) {
    if (!ctx || !desc || !out_buffer) {
        return lab_operation_result{LAB_ERROR_INVALID_PARAMETER, "Invalid parameters"};
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
    
    if (result.error == LAB_ERROR_NONE) {
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

} // extern "C"
