#ifndef LABFONT_RESOURCE_H
#define LABFONT_RESOURCE_H

#include "backend_types.h"
#include "internal_types.h"
#include <string>
#include <memory>

namespace labfont {

// Forward declarations
class Backend;

// Resource types
enum class ResourceType {
    Texture,
    Buffer,
    Font,
    RenderTarget
};

// Base resource class
class Resource {
public:
    virtual ~Resource() = default;
    
    ResourceType GetType() const { return m_type; }
    const std::string& GetName() const { return m_name; }
    bool IsValid() const { return m_valid; }

protected:
    Resource(ResourceType type, const std::string& name)
        : m_type(type), m_name(name), m_valid(false) {}
    
    void SetValid(bool valid) { m_valid = valid; }

private:
    ResourceType m_type;
    std::string m_name;
    bool m_valid;
};

// Texture resource
class TextureResource : public Resource {
public:
    TextureResource(const std::string& name, unsigned int width, unsigned int height, TextureFormat format)
        : Resource(ResourceType::Texture, name)
        , m_width(width)
        , m_height(height)
        , m_format(format) {}
    
    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }
    TextureFormat GetFormat() const { return m_format; }

private:
    unsigned int m_width;
    unsigned int m_height;
    TextureFormat m_format;
};

// Buffer resource
class BufferResource : public Resource {
public:
    BufferResource(const std::string& name, size_t size, bool dynamic)
        : Resource(ResourceType::Buffer, name)
        , m_size(size)
        , m_dynamic(dynamic) {}
    
    size_t GetSize() const { return m_size; }
    bool IsDynamic() const { return m_dynamic; }

private:
    size_t m_size;
    bool m_dynamic;
};

// Render target resource
class RenderTargetResource : public Resource {
public:
    RenderTargetResource(const std::string& name, unsigned int width, unsigned int height, 
                         TextureFormat format, bool hasDepth)
        : Resource(ResourceType::RenderTarget, name)
        , m_width(width)
        , m_height(height)
        , m_format(format)
        , m_hasDepth(hasDepth)
        , m_backendTarget(nullptr) {}
    
    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }
    TextureFormat GetFormat() const { return m_format; }
    bool HasDepth() const { return m_hasDepth; }
    
    // Set the backend render target
    void SetBackendTarget(std::shared_ptr<RenderTarget> target) { m_backendTarget = target; }
    
    // Get the backend render target
    RenderTarget* GetBackendTarget() const { return m_backendTarget.get(); }

private:
    unsigned int m_width;
    unsigned int m_height;
    TextureFormat m_format;
    bool m_hasDepth;
    std::shared_ptr<RenderTarget> m_backendTarget;
};

// Resource creation parameters
struct TextureParams {
    unsigned int width;
    unsigned int height;
    TextureFormat format;  // Changed from unsigned int to TextureFormat
    const void* data;      // Changed from initial_data to data to match usage
};

struct BufferParams {
    size_t size;
    bool dynamic;
    const void* data;      // Changed from initial_data to data to match usage
};

struct RenderTargetParams {
    unsigned int width;
    unsigned int height;
    TextureFormat format;
    bool hasDepth;
};

// Resource manager interface
class ResourceManager {
public:
    virtual ~ResourceManager() = default;

    virtual lab_result CreateTexture(const std::string& name,
                                     const TextureParams& params,
                                     std::shared_ptr<TextureResource>& out_texture) = 0;
    virtual lab_result CreateBuffer(const std::string& name,
                                    const BufferParams& params,
                                    std::shared_ptr<BufferResource>& out_buffer) = 0;
    virtual lab_result CreateRenderTarget(const std::string& name,
                                         const RenderTargetParams& params,
                                         std::shared_ptr<RenderTargetResource>& out_target) = 0;
    
    virtual void DestroyResource(const std::string& name) = 0;
    virtual std::shared_ptr<Resource> GetResource(const std::string& name) = 0;
};

} // namespace labfont

#endif // LABFONT_RESOURCE_H
