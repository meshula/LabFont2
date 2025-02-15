#ifndef LABFONT_CPU_BACKEND_H
#define LABFONT_CPU_BACKEND_H

#include "core/backend.h"
#include <vector>
#include <memory>

namespace labfont {

// CPU-based texture implementation
class CPUTexture : public Texture {
public:
    CPUTexture(const TextureDesc& desc)
        : m_width(desc.width)
        , m_height(desc.height)
        , m_format(desc.format)
        , m_renderTarget(desc.renderTarget)
        , m_readback(desc.readback)
    {
        size_t pixelSize = GetPixelSize(m_format);
        m_data.resize(m_width * m_height * pixelSize);
        
        if (desc.data) {
            memcpy(m_data.data(), desc.data, m_data.size());
        }
    }
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool IsRenderTarget() const override { return m_renderTarget; }
    bool SupportsReadback() const override { return m_readback; }
    
    // CPU-specific methods
    std::vector<uint8_t>& GetData() { return m_data; }
    const std::vector<uint8_t>& GetData() const { return m_data; }
    void SetData(const void* data, size_t size) {
        if (size <= m_data.size()) {
            memcpy(m_data.data(), data, size);
        }
    }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_renderTarget;
    bool m_readback;
    std::vector<uint8_t> m_data;
    
    static size_t GetPixelSize(TextureFormat format) {
        switch (format) {
            case TextureFormat::R8_UNORM: return 1;
            case TextureFormat::RG8_UNORM: return 2;
            case TextureFormat::RGBA8_UNORM: return 4;
            case TextureFormat::R16F: return 2;
            case TextureFormat::RG16F: return 4;
            case TextureFormat::RGBA16F: return 8;
            case TextureFormat::R32F: return 4;
            case TextureFormat::RG32F: return 8;
            case TextureFormat::RGBA32F: return 16;
            default: return 0;
        }
    }
};

// CPU-based render target implementation
class CPURenderTarget : public RenderTarget {
public:
    CPURenderTarget(const RenderTargetDesc& desc)
        : m_width(desc.width)
        , m_height(desc.height)
        , m_format(desc.format)
        , m_hasDepth(desc.hasDepth)
    {
        TextureDesc colorDesc = {
            .width = desc.width,
            .height = desc.height,
            .format = desc.format,
            .renderTarget = true,
            .readback = true,
            .data = nullptr
        };
        m_colorTexture = std::make_shared<CPUTexture>(colorDesc);
        
        if (m_hasDepth) {
            TextureDesc depthDesc = {
                .width = desc.width,
                .height = desc.height,
                .format = TextureFormat::R32F,
                .renderTarget = true,
                .readback = true,
                .data = nullptr
            };
            m_depthTexture = std::make_shared<CPUTexture>(depthDesc);
        }
    }
    
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    TextureFormat GetFormat() const override { return m_format; }
    bool HasDepth() const override { return m_hasDepth; }
    Texture* GetColorTexture() override { return m_colorTexture.get(); }
    Texture* GetDepthTexture() override { return m_depthTexture.get(); }
    
private:
    uint32_t m_width;
    uint32_t m_height;
    TextureFormat m_format;
    bool m_hasDepth;
    std::shared_ptr<CPUTexture> m_colorTexture;
    std::shared_ptr<CPUTexture> m_depthTexture;
};

// CPU-based backend implementation
class CPUBackend : public Backend {
public:
    CPUBackend() 
        : m_currentBlendMode(BlendMode::None)
    {}
    ~CPUBackend() override = default;
    
    lab_result Initialize(uint32_t width, uint32_t height) override {
        m_width = width;
        m_height = height;
        return {LAB_ERROR_NONE, nullptr};
    }
    
    lab_result Resize(uint32_t width, uint32_t height) override {
        m_width = width;
        m_height = height;
        return {LAB_ERROR_NONE, nullptr};
    }
    
    lab_result CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) override {
        auto texture = std::make_shared<CPUTexture>(desc);
        out_texture = texture;
        m_textures.push_back(texture);
        return {LAB_ERROR_NONE, nullptr};
    }
    
    lab_result UpdateTexture(Texture* texture, const void* data, size_t size) override {
        auto cpuTexture = static_cast<CPUTexture*>(texture);
        cpuTexture->SetData(data, size);
        return {LAB_ERROR_NONE, nullptr};
    }
    
    lab_result ReadbackTexture(Texture* texture, void* data, size_t size) override {
        auto cpuTexture = static_cast<CPUTexture*>(texture);
        const auto& textureData = cpuTexture->GetData();
        if (size <= textureData.size()) {
            memcpy(data, textureData.data(), size);
            return {LAB_ERROR_NONE, nullptr};
        }
        return {LAB_ERROR_INVALID_PARAMETER, "Buffer too small for readback"};
    }
    
    lab_result CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) override {
        auto target = std::make_shared<CPURenderTarget>(desc);
        out_target = target;
        m_renderTargets.push_back(target);
        return {LAB_ERROR_NONE, nullptr};
    }
    
    lab_result SetRenderTarget(RenderTarget* target) override {
        m_currentRenderTarget = target;
        return {LAB_ERROR_NONE, nullptr};
    }
    
    lab_result BeginFrame() override {
        m_commands.clear();
        return {LAB_ERROR_NONE, nullptr};
    }
    
    lab_result SubmitCommands(const std::vector<DrawCommand>& commands) override;
    
    lab_result EndFrame() override {
        return {LAB_ERROR_NONE, nullptr};
    }
    
    void DestroyTexture(Texture* texture) override {
        for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
            if (it->get() == texture) {
                m_textures.erase(it);
                break;
            }
        }
    }
    
    void DestroyRenderTarget(RenderTarget* target) override {
        for (auto it = m_renderTargets.begin(); it != m_renderTargets.end(); ++it) {
            if (it->get() == target) {
                m_renderTargets.erase(it);
                break;
            }
        }
    }
    
    size_t GetTextureMemoryUsage() const override {
        size_t total = 0;
        for (const auto& texture : m_textures) {
            auto cpuTexture = static_cast<CPUTexture*>(texture.get());
            total += cpuTexture->GetData().size();
        }
        return total;
    }
    
    size_t GetTotalMemoryUsage() const override {
        return GetTextureMemoryUsage();
    }
    
    bool SupportsTextureFormat(TextureFormat format) const override {
        return true;  // CPU backend supports all formats
    }
    
    bool SupportsBlendMode(BlendMode mode) const override {
        return true;  // CPU backend supports all blend modes
    }
    
    uint32_t GetMaxTextureSize() const override {
        return 16384;  // Reasonable default
    }
    
    // CPU-specific methods for testing
    const std::vector<DrawCommand>& GetSubmittedCommands() const {
        return m_commands;
    }
    
    RenderTarget* GetCurrentRenderTarget() const {
        return m_currentRenderTarget;
    }
    
    size_t GetTextureCount() const {
        return m_textures.size();
    }
    
    size_t GetRenderTargetCount() const {
        return m_renderTargets.size();
    }
    
private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<std::shared_ptr<Texture>> m_textures;
    std::vector<std::shared_ptr<RenderTarget>> m_renderTargets;
    std::vector<DrawCommand> m_commands;
    RenderTarget* m_currentRenderTarget = nullptr;
    BlendMode m_currentBlendMode;
};

} // namespace labfont

#endif // LABFONT_CPU_BACKEND_H
