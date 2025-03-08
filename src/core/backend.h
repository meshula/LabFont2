#ifndef LABFONT_BACKEND_H
#define LABFONT_BACKEND_H

#include "backend_types.h"
#include "internal_types.h"
#include "error.h"
#include <memory>
#include <vector>

namespace labfont {

// Forward declarations
class Texture;
class RenderTarget;

// Abstract backend interface
class Backend {
public:
    virtual ~Backend() = default;

    // Initialization
    virtual lab_result Initialize(uint32_t width, uint32_t height) = 0;
    virtual lab_result Resize(uint32_t width, uint32_t height) = 0;
    
    // Texture management
    virtual lab_result CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) = 0;
    virtual lab_result UpdateTexture(Texture* texture, const void* data, size_t size) = 0;
    virtual lab_result ReadbackTexture(Texture* texture, void* data, size_t size) = 0;
    
    // Render target management
    virtual lab_result CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) = 0;
    virtual lab_result SetRenderTarget(RenderTarget* target) = 0;
    
    // Draw command submission
    virtual lab_result BeginFrame() = 0;
    virtual lab_result SubmitCommands(const std::vector<DrawCommand>& commands) = 0;
    virtual lab_result EndFrame() = 0;
    
    // Resource cleanup
    virtual void DestroyTexture(Texture* texture) = 0;
    virtual void DestroyRenderTarget(RenderTarget* target) = 0;
    
    // Memory management
    virtual size_t GetTextureMemoryUsage() const = 0;
    virtual size_t GetTotalMemoryUsage() const = 0;
    
    // Backend capabilities
    virtual bool SupportsTextureFormat(lab_texture_format format) const = 0;
    virtual bool SupportsBlendMode(BlendMode mode) const = 0;
    virtual uint32_t GetMaxTextureSize() const = 0;
    
protected:
    Backend() = default;
    
    // Prevent copying
    Backend(const Backend&) = delete;
    Backend& operator=(const Backend&) = delete;
};

// Backend resource base classes
class Texture {
public:
    virtual ~Texture() = default;
    
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual lab_texture_format GetFormat() const = 0;
    virtual bool IsRenderTarget() const = 0;
    virtual bool SupportsReadback() const = 0;
    
protected:
    Texture() = default;
};

class RenderTarget {
public:
    virtual ~RenderTarget() = default;
    
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual lab_texture_format GetFormat() const = 0;
    virtual bool HasDepth() const = 0;
    virtual Texture* GetColorTexture() = 0;
    virtual Texture* GetDepthTexture() = 0;
    
protected:
    RenderTarget() = default;
};

// Backend factory function (to be implemented by each backend)
std::unique_ptr<Backend> CreateBackend(lab_backend_type type);

} // namespace labfont

#endif // LABFONT_BACKEND_H
