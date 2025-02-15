#include "wgpu_backend.h"
#include "wgpu_command_buffer.h"
#include <fstream>
#include <sstream>

namespace labfont {
namespace wgpu {

WGPUTexture::WGPUTexture(WGPUDevice* device, const TextureDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_renderTarget(desc.renderTarget)
    , m_readback(desc.readback)
    , m_texture(nullptr)
    , m_textureView(nullptr)
    , m_device(device)
{
    WGPUTextureDescriptor textureDesc = {};
    textureDesc.size = {desc.width, desc.height, 1};
    textureDesc.format = GetWGPUFormat(desc.format);
    textureDesc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    
    if (desc.renderTarget) {
        textureDesc.usage |= WGPUTextureUsage_RenderAttachment;
    }
    
    m_texture = wgpuDeviceCreateTexture(device->GetWGPUDevice(), &textureDesc);
    if (m_texture) {
        WGPUTextureViewDescriptor viewDesc = {};
        viewDesc.format = textureDesc.format;
        viewDesc.dimension = WGPUTextureViewDimension_2D;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.arrayLayerCount = 1;
        
        m_textureView = wgpuTextureCreateView(m_texture, &viewDesc);
        
        if (desc.data) {
            WGPUImageCopyTexture destination = {};
            destination.texture = m_texture;
            
            WGPUTextureDataLayout dataLayout = {};
            dataLayout.offset = 0;
            dataLayout.bytesPerRow = desc.width * 4;
            dataLayout.rowsPerImage = desc.height;
            
            WGPUExtent3D size = {desc.width, desc.height, 1};
            
            wgpuQueueWriteTexture(device->GetQueue(), &destination, desc.data, desc.width * desc.height * 4, &dataLayout, &size);
        }
    }
}

WGPUTexture::~WGPUTexture() {
    if (m_textureView) {
        wgpuTextureViewRelease(m_textureView);
    }
    if (m_texture) {
        wgpuTextureRelease(m_texture);
    }
}

WGPURenderTarget::WGPURenderTarget(WGPUDevice* device, const RenderTargetDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_hasDepth(desc.hasDepth)
    , m_device(device)
{
    // Create color texture
    TextureDesc colorDesc = {
        .width = desc.width,
        .height = desc.height,
        .format = desc.format,
        .renderTarget = true,
        .readback = true,
        .data = nullptr
    };
    m_colorTexture = std::make_shared<WGPUTexture>(device, colorDesc);
    
    // Create depth texture if needed
    if (desc.hasDepth) {
        TextureDesc depthDesc = {
            .width = desc.width,
            .height = desc.height,
            .format = TextureFormat::R32F,
            .renderTarget = true,
            .readback = false,
            .data = nullptr
        };
        m_depthTexture = std::make_shared<WGPUTexture>(device, depthDesc);
    }
    
    // Create render pass descriptor
    m_renderPassDesc = {};
    m_renderPassDesc.colorAttachment = m_colorTexture->GetWGPUTextureView();
    m_renderPassDesc.depthStencilAttachment = m_hasDepth ? m_depthTexture->GetWGPUTextureView() : nullptr;
    m_renderPassDesc.clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    m_renderPassDesc.clearDepth = 1.0f;
    m_renderPassDesc.clearStencil = 0;
}

WGPURenderTarget::~WGPURenderTarget() = default;

WGPUDevice::WGPUDevice()
    : m_device(nullptr)
    , m_queue(nullptr)
    , m_shaderModule(nullptr)
    , m_trianglePipeline(nullptr)
    , m_linePipeline(nullptr)
    , m_bindGroupLayout(nullptr)
{
}

WGPUDevice::~WGPUDevice() {
    if (m_bindGroupLayout) wgpuBindGroupLayoutRelease(m_bindGroupLayout);
    if (m_linePipeline) wgpuRenderPipelineRelease(m_linePipeline);
    if (m_trianglePipeline) wgpuRenderPipelineRelease(m_trianglePipeline);
    if (m_shaderModule) wgpuShaderModuleRelease(m_shaderModule);
    if (m_queue) wgpuQueueRelease(m_queue);
    if (m_device) wgpuDeviceRelease(m_device);
}

bool WGPUDevice::Initialize() {
    // Create adapter
    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.powerPreference = WGPUPowerPreference_HighPerformance;
    
    WGPUAdapter adapter = nullptr;
    wgpuInstanceRequestAdapter(nullptr, &adapterOpts,
        [](WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message, void* userdata) {
            if (status == WGPURequestAdapterStatus_Success) {
                *static_cast<WGPUAdapter*>(userdata) = adapter;
            }
        }, &adapter);
    
    if (!adapter) {
        return false;
    }
    
    // Create device
    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.requiredFeatures = nullptr;
    deviceDesc.requiredLimits = nullptr;
    deviceDesc.defaultQueue.nextInChain = nullptr;
    
    m_device = wgpuAdapterCreateDevice(adapter, &deviceDesc);
    wgpuAdapterRelease(adapter);
    
    if (!m_device) {
        return false;
    }
    
    // Get queue
    m_queue = wgpuDeviceGetQueue(m_device);
    if (!m_queue) {
        return false;
    }
    
    // Load shaders
    if (!LoadShaders()) {
        return false;
    }
    
    // Create pipeline states
    if (!CreatePipelineStates()) {
        return false;
    }
    
    return true;
}

bool WGPUDevice::LoadShaders() {
    // Read shader source
    std::ifstream file("src/backends/wgpu/shaders/primitives.wgsl");
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    // Create shader module
    WGPUShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    wgslDesc.code = source.c_str();
    
    WGPUShaderModuleDescriptor moduleDesc = {};
    moduleDesc.nextInChain = &wgslDesc.chain;
    
    m_shaderModule = wgpuDeviceCreateShaderModule(m_device, &moduleDesc);
    return m_shaderModule != nullptr;
}

bool WGPUDevice::CreatePipelineStates() {
    // Create bind group layout
    WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
    m_bindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_device, &bindGroupLayoutDesc);
    if (!m_bindGroupLayout) {
        return false;
    }
    
    // Create pipeline layout
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &m_bindGroupLayout;
    
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(m_device, &pipelineLayoutDesc);
    if (!pipelineLayout) {
        return false;
    }
    
    // Create vertex state
    WGPUVertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.arrayStride = sizeof(WGPUVertex);
    vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
    vertexBufferLayout.attributeCount = 3;
    
    WGPUVertexAttribute attributes[3] = {};
    // Position
    attributes[0].format = WGPUVertexFormat_Float32x2;
    attributes[0].offset = offsetof(WGPUVertex, position);
    attributes[0].shaderLocation = 0;
    // Texcoord
    attributes[1].format = WGPUVertexFormat_Float32x2;
    attributes[1].offset = offsetof(WGPUVertex, texcoord);
    attributes[1].shaderLocation = 1;
    // Color
    attributes[2].format = WGPUVertexFormat_Float32x4;
    attributes[2].offset = offsetof(WGPUVertex, color);
    attributes[2].shaderLocation = 2;
    
    vertexBufferLayout.attributes = attributes;
    
    WGPUVertexState vertexState = {};
    vertexState.module = m_shaderModule;
    vertexState.entryPoint = "vertex_main";
    vertexState.bufferCount = 1;
    vertexState.buffers = &vertexBufferLayout;
    
    // Create fragment state
    WGPUFragmentState fragmentState = {};
    fragmentState.module = m_shaderModule;
    fragmentState.entryPoint = "fragment_main";
    fragmentState.targetCount = 1;
    
    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_RGBA8Unorm;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    
    fragmentState.targets = &colorTarget;
    
    // Create pipeline descriptor
    WGPURenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDesc.primitive.cullMode = WGPUCullMode_None;
    
    // Create triangle pipeline
    m_trianglePipeline = wgpuDeviceCreateRenderPipeline(m_device, &pipelineDesc);
    if (!m_trianglePipeline) {
        wgpuPipelineLayoutRelease(pipelineLayout);
        return false;
    }
    
    // Create line pipeline
    fragmentState.entryPoint = "fragment_line";
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
    
    m_linePipeline = wgpuDeviceCreateRenderPipeline(m_device, &pipelineDesc);
    
    wgpuPipelineLayoutRelease(pipelineLayout);
    return m_linePipeline != nullptr;
}

WGPUBackend::WGPUBackend()
    : m_device(std::make_unique<WGPUDevice>())
    , m_currentBlendMode(BlendMode::None)
{
}

WGPUBackend::~WGPUBackend() = default;

lab_result WGPUBackend::Initialize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    
    if (!m_device->Initialize()) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to initialize WebGPU device"};
    }
    
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::Resize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) {
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
    WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
    wgpuQueueSubmit(m_device->GetQueue(), 1, &cmdBuffer);
    
    // Map buffer and copy data
    wgpuBufferMapAsync(stagingBuffer, WGPUMapMode_Read, 0, size,
        [](WGPUBufferMapAsyncStatus status, void* userdata) {
            *static_cast<bool*>(userdata) = (status == WGPUBufferMapAsyncStatus_Success);
        }, nullptr);
    
    bool mapped = false;
    while (!mapped) {
        wgpuDeviceTick(m_device->GetWGPUDevice());
    }
    
    void* mappedData = wgpuBufferGetMappedRange(stagingBuffer, 0, size);
    std::memcpy(data, mappedData, size);
    
    wgpuBufferUnmap(stagingBuffer);
    wgpuBufferRelease(stagingBuffer);
    wgpuCommandBufferRelease(cmdBuffer);
    wgpuCommandEncoderRelease(encoder);
    
    return {LAB_ERROR_NONE, nullptr};
}

lab_result WGPUBackend::CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) {
    auto target = std::make_shared<WGPURenderTarget>(m_device.get(), desc);
    if (!target->GetColorTexture()) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to create WebGPU render target"};
    }
    
    out_target = target;
    m_renderTargets.push_back(target);
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
