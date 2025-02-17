#ifndef LABFONT_RESOURCE_H
#define LABFONT_RESOURCE_H

#include "backend_types.h"
#include <string>
#include <memory>

namespace labfont {

// Forward declarations
class Backend;

// Resource types
enum class ResourceType {
    Texture,
    Buffer,
    Font
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
    TextureResource(const std::string& name, unsigned int width, unsigned int height, unsigned int format)
        : Resource(ResourceType::Texture, name)
        , m_width(width)
        , m_height(height)
        , m_format(format) {}
    
    unsigned int GetWidth() const { return m_width; }
    unsigned int GetHeight() const { return m_height; }
    unsigned int GetFormat() const { return m_format; }

private:
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_format;
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

// Resource creation parameters
struct TextureParams {
    unsigned int width;
    unsigned int height;
    unsigned int format;
    const void* initial_data;
};

struct BufferParams {
    size_t size;
    bool dynamic;
    const void* initial_data;
};

// Resource manager interface
class ResourceManager {
public:
    virtual ~ResourceManager() = default;

    virtual labfont::lab_result CreateTexture(const std::string& name, const TextureParams& params, std::shared_ptr<TextureResource>& out_texture) = 0;
    virtual labfont::lab_result CreateBuffer(const std::string& name, const BufferParams& params, std::shared_ptr<BufferResource>& out_buffer) = 0;
    
    virtual void DestroyResource(const std::string& name) = 0;
    virtual std::shared_ptr<Resource> GetResource(const std::string& name) = 0;
};

} // namespace labfont

#endif // LABFONT_RESOURCE_H
