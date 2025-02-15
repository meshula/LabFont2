#include "resource_manager.h"
#include "backend.h"
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
