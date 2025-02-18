#include "wgpu_backend.h"
#include "wgpu_command_buffer.h"
#include <cstring>
#include <emscripten.h>

namespace labfont {
namespace wgpu {

WGPUBackend::WGPUBackend()
    : m_width(0)
    , m_height(0)
    , m_device(nullptr)
    , m_currentRenderTarget(nullptr)
    , m_currentBlendMode(BlendMode::None)
{
}

WGPUBackend::~WGPUBackend() = default;

lab_result WGPUBackend::Initialize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    
    // Get the WebGPU device that was initialized by JavaScript
    bool hasDevice = EM_ASM_INT({
        if (!Module.gpuDevice) {
            console.error("WebGPU device not initialized. Make sure to initialize WebGPU in JavaScript first.");
            return 0;
        }
        return 1;
    });
    
    if (!hasDevice) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "WebGPU device not initialized"};
    }
    
    m_device = std::make_unique<WGPUDevice>();
    m_device->device = emscripten_webgpu_get_device();
    if (!m_device->device) {
        EM_ASM({
            console.error("Failed to get WebGPU device from Emscripten");
            console.log("Module.gpuDevice:", Module.gpuDevice);
        });
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to get WebGPU device from browser"};
    }
    
    EM_ASM({
        console.log("Successfully got WebGPU device from Emscripten");
    });
    
    // Get queue
    m_device->queue = wgpuDeviceGetQueue(m_device->device);
    if (!m_device->queue) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to get WebGPU queue"};
    }
    
    // Create shader module from WGSL
    WGPUShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    wgslDesc.code = R"(
        // Vertex shader input
        struct VertexInput {
            @location(0) position: vec2<f32>,
            @location(1) texcoord: vec2<f32>,
            @location(2) color: vec4<f32>,
        };

        // Vertex shader output / Fragment shader input
        struct VertexOutput {
            @builtin(position) position: vec4<f32>,
            @location(0) texcoord: vec2<f32>,
            @location(1) color: vec4<f32>,
        };

        // Vertex shader
        @vertex
        fn vertex_main(input: VertexInput) -> VertexOutput {
            var output: VertexOutput;
            output.position = vec4<f32>(input.position, 0.0, 1.0);
            output.texcoord = input.texcoord;
            output.color = input.color;
            return output;
        }

        // Fragment shader for triangles
        @fragment
        fn fragment_main(input: VertexOutput) -> @location(0) vec4<f32> {
            return input.color;
        }

        // Fragment shader for lines with anti-aliasing
        @fragment
        fn fragment_line(input: VertexOutput) -> @location(0) vec4<f32> {
            let dist = abs(input.texcoord.y - 0.5) * 2.0;
            let alpha = 1.0 - smoothstep(0.8, 1.0, dist);
            return vec4<f32>(input.color.rgb, input.color.a * alpha);
        }
    )";
    
    WGPUShaderModuleDescriptor moduleDesc = {};
    moduleDesc.nextInChain = &wgslDesc.chain;
    
    m_device->shaderModule = wgpuDeviceCreateShaderModule(m_device->device, &moduleDesc);
    if (!m_device->shaderModule) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to create shader module"};
    }
    
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::Resize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::CreateTexture(const labfont::TextureDesc& desc, std::shared_ptr<Texture>& out_texture) {
    auto texture = std::make_shared<WGPUTexture>(m_device.get(), desc);
    if (!texture->GetWGPUTexture()) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to create WebGPU texture"};
    }
    
    out_texture = texture;
    m_textures.push_back(texture);
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::UpdateTexture(Texture* texture, const void* data, size_t size) {
    auto wgpuTexture = static_cast<WGPUTexture*>(texture);
    
    WGPUImageCopyTexture destination = {};
    destination.texture = wgpuTexture->GetWGPUTexture();
    
    WGPUTextureDataLayout dataLayout = {};
    dataLayout.offset = 0;
    dataLayout.bytesPerRow = texture->GetWidth() * 4;
    dataLayout.rowsPerImage = texture->GetHeight();
    
    WGPUExtent3D copySize = {texture->GetWidth(), texture->GetHeight(), 1};
    
    wgpuQueueWriteTexture(m_device->GetQueue(), &destination, data, size, &dataLayout, &copySize);
    
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::ReadbackTexture(Texture* texture, void* data, size_t size) {
    auto wgpuTexture = static_cast<WGPUTexture*>(texture);
    
    // Create staging buffer
    WGPUBufferDescriptor bufferDesc = {};
    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead;
    
    WGPUBuffer stagingBuffer = wgpuDeviceCreateBuffer(m_device->GetWGPUDevice(), &bufferDesc);
    if (!stagingBuffer) {
        return {LAB_ERROR_OUT_OF_MEMORY, "Failed to create staging buffer"};
    }
    
    // Copy texture to buffer
    WGPUImageCopyTexture source = {};
    source.texture = wgpuTexture->GetWGPUTexture();
    
    WGPUImageCopyBuffer destination = {};
    destination.buffer = stagingBuffer;
    destination.layout.offset = 0;
    destination.layout.bytesPerRow = texture->GetWidth() * 4;
    destination.layout.rowsPerImage = texture->GetHeight();
    
    WGPUExtent3D copySize = {texture->GetWidth(), texture->GetHeight(), 1};
    
    WGPUCommandEncoderDescriptor encoderDesc = {};
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_device->GetWGPUDevice(), &encoderDesc);
    wgpuCommandEncoderCopyTextureToBuffer(encoder, &source, &destination, &copySize);
    
    WGPUCommandBufferDescriptor cmdBufferDesc = {};
    WGPUCommandBufferRef cmdBuffer = wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
    wgpuQueueSubmit(m_device->GetQueue(), 1, &cmdBuffer);
    
    // Map buffer and copy data
    wgpuBufferMapAsync(stagingBuffer, WGPUMapMode_Read, 0, size,
        [](WGPUBufferMapAsyncStatus status, void* userdata) {
            *static_cast<bool*>(userdata) = (status == WGPUBufferMapAsyncStatus_Success);
        }, nullptr);
    
    bool mapped = false;
    while (!mapped) {
        emscripten_sleep(1);
    }
    
    void* mappedData = wgpuBufferGetMappedRange(stagingBuffer, 0, size);
    std::memcpy(data, mappedData, size);
    
    wgpuBufferUnmap(stagingBuffer);
    wgpuBufferRelease(stagingBuffer);
    wgpuCommandEncoderRelease(encoder);
    
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::CreateRenderTarget(const labfont::RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) {
    auto wgpuTarget = std::make_shared<WGPURenderTarget>(m_device.get(), desc);
    if (!wgpuTarget->GetColorTexture()) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to create WebGPU render target"};
    }
    
    out_target = std::static_pointer_cast<RenderTarget>(wgpuTarget);
    m_renderTargets.push_back(out_target);
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::SetRenderTarget(RenderTarget* target) {
    m_currentRenderTarget = target;
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::BeginFrame() {
    m_currentCommandBuffer = std::make_unique<WGPUCommandBuffer>(m_device.get());
    if (!m_currentCommandBuffer->Begin()) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to create WebGPU command buffer"};
    }
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::SubmitCommands(const std::vector<DrawCommand>& commands) {
    if (!m_currentRenderTarget) {
        return {LAB_ERROR_INVALID_STATE, "No render target set"};
    }
    
    auto wgpuTarget = static_cast<WGPURenderTarget*>(m_currentRenderTarget);
    if (!m_currentCommandBuffer->BeginRenderPass(wgpuTarget)) {
        return {LAB_ERROR_INVALID_STATE, "Failed to begin render pass"};
    }
    
    for (const auto& cmd : commands) {
        switch (cmd.type) {
            case DrawCommandType::Clear: {
                m_currentCommandBuffer->Clear(cmd.clear.color);
                break;
            }
            
            case DrawCommandType::DrawTriangles: {
                const auto& params = cmd.triangles;
                m_currentCommandBuffer->DrawTriangles(params.vertices, params.vertexCount);
                break;
            }
            
            case DrawCommandType::DrawLines: {
                const auto& params = cmd.lines;
                m_currentCommandBuffer->DrawLines(params.vertices, params.vertexCount, params.lineWidth);
                break;
            }
            
            case DrawCommandType::SetBlendMode: {
                m_currentBlendMode = cmd.blend.mode;
                m_currentCommandBuffer->SetBlendMode(cmd.blend.mode);
                break;
            }
            
            case DrawCommandType::SetScissor: {
                const auto& params = cmd.scissor;
                m_currentCommandBuffer->SetScissorRect(
                    params.x, params.y,
                    params.width, params.height
                );
                break;
            }
            
            case DrawCommandType::SetViewport: {
                const auto& params = cmd.viewport;
                m_currentCommandBuffer->SetViewport(
                    params.x, params.y,
                    params.width, params.height
                );
                break;
            }
        }
    }
    
    m_currentCommandBuffer->EndRenderPass();
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::EndFrame() {
    if (!m_currentCommandBuffer->End()) {
        return {LAB_ERROR_INVALID_STATE, "Failed to end command buffer"};
    }
    m_currentCommandBuffer.reset();
    return {LAB_ERROR_NONE, nullptr};
}

void WGPUBackend::DestroyTexture(Texture* texture) {
    for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
        if (it->get() == texture) {
            m_textures.erase(it);
            break;
        }
    }
}

void WGPUBackend::DestroyRenderTarget(RenderTarget* target) {
    for (auto it = m_renderTargets.begin(); it != m_renderTargets.end(); ++it) {
        if (it->get() == target) {
            m_renderTargets.erase(it);
            break;
        }
    }
}

size_t WGPUBackend::GetTextureMemoryUsage() const {
    // TODO: Implement texture memory tracking
    return 0;
}

size_t WGPUBackend::GetTotalMemoryUsage() const {
    return GetTextureMemoryUsage();
}

bool WGPUBackend::SupportsTextureFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::R8_UNORM:
        case TextureFormat::RG8_UNORM:
        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::R16F:
        case TextureFormat::RG16F:
        case TextureFormat::RGBA16F:
        case TextureFormat::R32F:
        case TextureFormat::RG32F:
        case TextureFormat::RGBA32F:
            return true;
        default:
            return false;
    }
}

bool WGPUBackend::SupportsBlendMode(BlendMode mode) const {
    return true;  // WebGPU supports all blend modes
}

uint32_t WGPUBackend::GetMaxTextureSize() const {
    return 8192;  // WebGPU minimum guaranteed size
}

} // namespace wgpu
} // namespace labfont
