#ifndef LABFONT_RESOURCE_MANAGER_IMPL_H
#define LABFONT_RESOURCE_MANAGER_IMPL_H

#include "resource.h"
#include "error.h"
#include <unordered_map>
#include <mutex>

namespace labfont {

class ResourceManagerImpl : public ResourceManager {
public:
    ResourceManagerImpl(Backend* backend);
    ~ResourceManagerImpl() override;

    lab_result CreateTexture(const std::string& name, const TextureParams& params, std::shared_ptr<TextureResource>& out_texture) override;
    lab_result CreateBuffer(const std::string& name, const BufferParams& params, std::shared_ptr<BufferResource>& out_buffer) override;
    lab_result CreateRenderTarget(const std::string& name, const RenderTargetParams& params, std::shared_ptr<RenderTargetResource>& out_target) override;
    
    void DestroyResource(const std::string& name) override;
    std::shared_ptr<Resource> GetResource(const std::string& name) override;

private:
    Backend* m_backend;  // Non-owning pointer to backend
    std::unordered_map<std::string, std::shared_ptr<Resource>> m_resources;
    std::mutex m_mutex;  // Protect resource map access

    // Helper methods
    bool ResourceExists(const std::string& name);
    void RemoveResource(const std::string& name);
};

} // namespace labfont

#endif // LABFONT_RESOURCE_MANAGER_IMPL_H
