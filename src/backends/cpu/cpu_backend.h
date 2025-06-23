#ifndef LABFONT_CPU_BACKEND_H
#define LABFONT_CPU_BACKEND_H

#include "labfont/labfont_types.h"
#include "core/backend.h"
#include <iostream>
#include <vector>

namespace labfont {

class CPUTexture : public Texture {
public:
    CPUTexture(const TextureDesc& desc)
        : m_width(desc.width)
        , m_height(desc.height)
        , m_format(desc.format)
        , m_renderTarget(desc.renderTarget)
        , m_readback(desc.readback)
        , m_data(desc.width * desc.height * 4) // Always RGBA8
    {
        if (desc.data && desc.dataSize > 0) {
            memcpy(m_data.data(), desc.data, desc.dataSize);
        }
    }
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    lab_texture_format GetFormat() const override { return m_format; }
    bool IsRenderTarget() const override { return m_renderTarget; }
    bool SupportsReadback() const override { return m_readback; }
    
    // CPU-specific methods
    uint8_t* GetData() { return m_data.data(); }
    const uint8_t* GetData() const { return m_data.data(); }
    void SetData(const void* data, size_t size) {
        if (size <= m_data.size()) {
            memcpy(m_data.data(), data, size);
        }
    }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    lab_texture_format m_format;
    bool m_renderTarget;
    bool m_readback;
    std::vector<uint8_t> m_data;
};

class CPURenderTarget : public RenderTarget {
public:
    CPURenderTarget(const RenderTargetDesc& desc)
        : m_width(desc.width)
        , m_height(desc.height)
        , m_format(desc.format)
        , m_hasDepth(desc.hasDepth)
    {
        // Create color texture
        TextureDesc colorDesc = {
            .width = desc.width,
            .height = desc.height,
            .format = desc.format,
            .data = nullptr,
            .renderTarget = true,
            .readback = true,
            .dataSize = 0
        };
        m_colorTexture = std::make_shared<CPUTexture>(colorDesc);
        
        // Create depth texture if needed
        if (m_hasDepth) {
            TextureDesc depthDesc = {
                .width = desc.width,
                .height = desc.height,
                .format = LAB_TEXTURE_FORMAT_D32F,
                .data = nullptr,
                .renderTarget = true,
                .readback = true,
                .dataSize = 0
            };
            m_depthTexture = std::make_shared<CPUTexture>(depthDesc);
        }
    }
    
    virtual ~CPURenderTarget() {
        std::cout << "destructing CPURenderTarget\n";
    }
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    lab_texture_format GetFormat() const override { return m_format; }
    bool HasDepth() const override { return m_hasDepth; }
    
    Texture* GetColorTexture() override { return m_colorTexture.get(); }
    Texture* GetDepthTexture() override { return m_depthTexture.get(); }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    lab_texture_format m_format;
    bool m_hasDepth;
    std::shared_ptr<CPUTexture> m_colorTexture;
    std::shared_ptr<CPUTexture> m_depthTexture;
};

class CPUBackend : public Backend {
public:
    CPUBackend() = default;
    ~CPUBackend() = default;
    
    lab_result Initialize(uint32_t width, uint32_t height) override {
        // Validate parameters
        if (width == 0 || height == 0) {
            return LAB_RESULT_INVALID_DIMENSION;
        }
        return LAB_RESULT_OK;
    }
    
    lab_result Resize(uint32_t width, uint32_t height) override {
        return LAB_RESULT_OK;
    }
    
    lab_result CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) override {
        try {
            out_texture = std::make_shared<CPUTexture>(desc);
            return LAB_RESULT_OK;
        }
        catch (const std::exception& e) {
            return LAB_RESULT_OUT_OF_MEMORY;
        }
    }
    
    lab_result UpdateTexture(Texture* texture, const void* data, size_t size) override {
        auto cpuTexture = static_cast<CPUTexture*>(texture);
        if (!cpuTexture) {
            return LAB_RESULT_INVALID_TEXTURE;
        }
        cpuTexture->SetData(data, size);
        return LAB_RESULT_OK;
    }
    
    lab_result ReadbackTexture(Texture* texture, void* data, size_t size) override {
        auto cpuTexture = static_cast<CPUTexture*>(texture);
        if (!cpuTexture) {
            return LAB_RESULT_INVALID_TEXTURE;
        }
        if (!cpuTexture || !cpuTexture->SupportsReadback()) {
            return LAB_RESULT_READBACK_NOT_SUPPORTED;
        }
        memcpy(data, cpuTexture->GetData(), size);
        return LAB_RESULT_OK;
    }
    
    lab_result CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) override {
        try {
            out_target = std::make_shared<CPURenderTarget>(desc);
            return LAB_RESULT_OK;
        } catch (const std::exception& e) {
            return LAB_RESULT_OUT_OF_MEMORY;
        }
    }
    
    lab_result SetRenderTarget(RenderTarget* target) override {
        m_currentRenderTarget = static_cast<CPURenderTarget*>(target);
        return LAB_RESULT_OK;
    }
    
    lab_result BeginFrame() override {
        return LAB_RESULT_OK;
    }
    
    lab_result SubmitCommands(const std::vector<DrawCommand>& commands) override;
    
    lab_result EndFrame() override {
        return LAB_RESULT_OK;
    }
    
    void DestroyTexture(Texture* texture) override {}
    void DestroyRenderTarget(RenderTarget* target) override {
        // The actual resource will be destroyed when the shared_ptr goes out of scope
    }
    
    size_t GetTextureMemoryUsage() const override { return 0; }
    size_t GetTotalMemoryUsage() const override { return 0; }
    
    bool SupportsTextureFormat(lab_texture_format format) const override {
        return true; // CPU backend supports all formats
    }
    
    bool SupportsBlendMode(BlendMode mode) const override {
        return true; // CPU backend supports all blend modes
    }
    
    uint32_t GetMaxTextureSize() const override {
        return 8192; // Arbitrary limit
    }
    
    // For testing
    const std::vector<DrawCommand>& GetCommands() const { return m_commands; }
    void ClearCommands() { m_commands.clear(); }
    
private:
    CPURenderTarget* m_currentRenderTarget = nullptr;
    BlendMode m_currentBlendMode = BlendMode::Alpha;
    std::vector<DrawCommand> m_commands;
    
    // Viewport state for coordinate transformation
    float m_viewportX = 0.0f;
    float m_viewportY = 0.0f;
    float m_viewportWidth = 1.0f;
    float m_viewportHeight = 1.0f;
    
    // Transform normalized coordinates (-1,1) to viewport coordinates (0,1)
    void TransformVertex(lab_vertex_2TC& vertex) {
        // Convert from normalized device coordinates to viewport coordinates
        // Input: vertex in -1,1 range (standard graphics coordinates)
        // Output: vertex in 0,1 range (LabFont coordinate system)
        vertex.position[0] = (vertex.position[0] + 1.0f) * 0.5f;
        vertex.position[1] = (vertex.position[1] + 1.0f) * 0.5f;
        
        // Apply viewport transformation
        vertex.position[0] = m_viewportX + vertex.position[0] * m_viewportWidth;
        vertex.position[1] = m_viewportY + vertex.position[1] * m_viewportHeight;
    };
};

} // namespace labfont

#endif // LABFONT_CPU_BACKEND_H
