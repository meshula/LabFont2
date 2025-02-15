#import "metal_backend.h"
#import "metal_command_buffer.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace labfont {
namespace metal {

MetalTexture::MetalTexture(MetalDevice* device, const TextureDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_renderTarget(desc.renderTarget)
    , m_readback(desc.readback)
    , m_texture(nil)
    , m_device(device)
{
    MTLTextureDescriptor* textureDesc = [[MTLTextureDescriptor alloc] init];
    textureDesc.width = desc.width;
    textureDesc.height = desc.height;
    textureDesc.pixelFormat = MTLPixelFormatRGBA8Unorm; // TODO: Map format
    textureDesc.usage = MTLTextureUsageShaderRead;
    
    if (desc.renderTarget) {
        textureDesc.usage |= MTLTextureUsageRenderTarget;
    }
    
    m_texture = [device->GetMTLDevice() newTextureWithDescriptor:textureDesc];
    [textureDesc release];
    
    if (desc.data) {
        [m_texture replaceRegion:MTLRegionMake2D(0, 0, desc.width, desc.height)
                    mipmapLevel:0
                    withBytes:desc.data
                    bytesPerRow:desc.width * 4];
    }
}

MetalTexture::~MetalTexture() {
    if (m_texture) {
        [m_texture release];
    }
}

MetalRenderTarget::MetalRenderTarget(MetalDevice* device, const RenderTargetDesc& desc)
    : m_width(desc.width)
    , m_height(desc.height)
    , m_format(desc.format)
    , m_hasDepth(desc.hasDepth)
    , m_renderPassDescriptor(nil)
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
    m_colorTexture = std::make_shared<MetalTexture>(device, colorDesc);
    
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
        m_depthTexture = std::make_shared<MetalTexture>(device, depthDesc);
    }
    
    // Create render pass descriptor
    m_renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
    m_renderPassDescriptor.colorAttachments[0].texture = m_colorTexture->GetMTLTexture();
    m_renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
    m_renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
    
    if (m_hasDepth) {
        m_renderPassDescriptor.depthAttachment.texture = m_depthTexture->GetMTLTexture();
        m_renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        m_renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
        m_renderPassDescriptor.depthAttachment.clearDepth = 1.0;
    }
}

MetalRenderTarget::~MetalRenderTarget() {
    if (m_renderPassDescriptor) {
        [m_renderPassDescriptor release];
    }
}

MetalDevice::MetalDevice()
    : m_device(nil)
    , m_commandQueue(nil)
    , m_shaderLibrary(nil)
    , m_trianglePipeline(nil)
    , m_linePipeline(nil)
    , m_depthState(nil)
{
}

MetalDevice::~MetalDevice() {
    if (m_depthState) [m_depthState release];
    if (m_linePipeline) [m_linePipeline release];
    if (m_trianglePipeline) [m_trianglePipeline release];
    if (m_shaderLibrary) [m_shaderLibrary release];
    if (m_commandQueue) [m_commandQueue release];
    if (m_device) [m_device release];
}

bool MetalDevice::Initialize() {
    // Create Metal device
    m_device = MTLCreateSystemDefaultDevice();
    if (!m_device) {
        return false;
    }
    
    // Create command queue
    m_commandQueue = [m_device newCommandQueue];
    if (!m_commandQueue) {
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

bool MetalDevice::LoadShaders() {
    NSString* libraryPath = [[NSBundle mainBundle] pathForResource:@"primitives" ofType:@"metallib"];
    if (!libraryPath) {
        return false;
    }
    
    NSURL* libraryURL = [NSURL fileURLWithPath:libraryPath];
    NSError* error = nil;
    m_shaderLibrary = [m_device newLibraryWithURL:libraryURL error:&error];
    if (!m_shaderLibrary) {
        return false;
    }
    
    return true;
}

bool MetalDevice::CreatePipelineStates() {
    // Create vertex descriptor
    MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
    
    // Position
    vertexDesc.attributes[0].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[0].offset = offsetof(MetalVertex, position);
    vertexDesc.attributes[0].bufferIndex = 0;
    
    // Texcoord
    vertexDesc.attributes[1].format = MTLVertexFormatFloat2;
    vertexDesc.attributes[1].offset = offsetof(MetalVertex, texcoord);
    vertexDesc.attributes[1].bufferIndex = 0;
    
    // Color
    vertexDesc.attributes[2].format = MTLVertexFormatFloat4;
    vertexDesc.attributes[2].offset = offsetof(MetalVertex, color);
    vertexDesc.attributes[2].bufferIndex = 0;
    
    vertexDesc.layouts[0].stride = sizeof(MetalVertex);
    vertexDesc.layouts[0].stepRate = 1;
    vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    // Create render pipeline descriptor
    MTLRenderPipelineDescriptor* pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexDescriptor = vertexDesc;
    pipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatRGBA8Unorm;
    pipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    
    // Triangle pipeline
    id<MTLFunction> vertexFunc = [m_shaderLibrary newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunc = [m_shaderLibrary newFunctionWithName:@"fragment_main"];
    
    pipelineDesc.vertexFunction = vertexFunc;
    pipelineDesc.fragmentFunction = fragmentFunc;
    
    NSError* error = nil;
    m_trianglePipeline = [m_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    
    [vertexFunc release];
    [fragmentFunc release];
    
    if (!m_trianglePipeline) {
        [vertexDesc release];
        [pipelineDesc release];
        return false;
    }
    
    // Line pipeline
    fragmentFunc = [m_shaderLibrary newFunctionWithName:@"fragment_line"];
    pipelineDesc.fragmentFunction = fragmentFunc;
    
    m_linePipeline = [m_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    
    [fragmentFunc release];
    [vertexDesc release];
    [pipelineDesc release];
    
    if (!m_linePipeline) {
        return false;
    }
    
    // Create depth state
    MTLDepthStencilDescriptor* depthDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
    depthDesc.depthWriteEnabled = YES;
    
    m_depthState = [m_device newDepthStencilStateWithDescriptor:depthDesc];
    [depthDesc release];
    
    if (!m_depthState) {
        return false;
    }
    
    return true;
}

MetalBackend::MetalBackend()
    : m_device(std::make_unique<MetalDevice>())
    , m_currentBlendMode(BlendMode::None)
{
}

MetalBackend::~MetalBackend() = default;

lab_result MetalBackend::Initialize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    
    if (!m_device->Initialize()) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to initialize Metal device"};
    }
    
    return {LAB_ERROR_NONE, nullptr};
}

lab_result MetalBackend::Resize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return {LAB_ERROR_NONE, nullptr};
}

lab_result MetalBackend::CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) {
    auto texture = std::make_shared<MetalTexture>(m_device.get(), desc);
    if (!texture->GetMTLTexture()) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to create Metal texture"};
    }
    
    out_texture = texture;
    m_textures.push_back(texture);
    return {LAB_ERROR_NONE, nullptr};
}

lab_result MetalBackend::UpdateTexture(Texture* texture, const void* data, size_t size) {
    auto metalTexture = static_cast<MetalTexture*>(texture);
    auto mtlTexture = metalTexture->GetMTLTexture();
    
    [mtlTexture replaceRegion:MTLRegionMake2D(0, 0, texture->GetWidth(), texture->GetHeight())
                mipmapLevel:0
                withBytes:data
                bytesPerRow:texture->GetWidth() * 4];
    
    return {LAB_ERROR_NONE, nullptr};
}

lab_result MetalBackend::ReadbackTexture(Texture* texture, void* data, size_t size) {
    auto metalTexture = static_cast<MetalTexture*>(texture);
    auto mtlTexture = metalTexture->GetMTLTexture();
    
    MTLRegion region = MTLRegionMake2D(0, 0, texture->GetWidth(), texture->GetHeight());
    NSUInteger bytesPerRow = texture->GetWidth() * 4;
    [mtlTexture getBytes:data
             bytesPerRow:bytesPerRow
              fromRegion:region
             mipmapLevel:0];
    
    return {LAB_ERROR_NONE, nullptr};
}

lab_result MetalBackend::CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) {
    auto target = std::make_shared<MetalRenderTarget>(m_device.get(), desc);
    if (!target->GetRenderPassDescriptor()) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to create Metal render target"};
    }
    
    out_target = target;
    m_renderTargets.push_back(target);
    return {LAB_ERROR_NONE, nullptr};
}

lab_result MetalBackend::SetRenderTarget(RenderTarget* target) {
    m_currentRenderTarget = target;
    return {LAB_ERROR_NONE, nullptr};
}

lab_result MetalBackend::BeginFrame() {
    m_currentCommandBuffer = std::make_unique<MetalCommandBuffer>(m_device.get());
    if (!m_currentCommandBuffer->Begin()) {
        return {LAB_ERROR_INITIALIZATION_FAILED, "Failed to create Metal command buffer"};
    }
    return {LAB_ERROR_NONE, nullptr};
}

lab_result MetalBackend::SubmitCommands(const std::vector<DrawCommand>& commands) {
    if (!m_currentRenderTarget) {
        return {LAB_ERROR_INVALID_STATE, "No render target set"};
    }
    
    auto metalTarget = static_cast<MetalRenderTarget*>(m_currentRenderTarget);
    if (!m_currentCommandBuffer->BeginRenderPass(metalTarget)) {
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

lab_result MetalBackend::EndFrame() {
    if (!m_currentCommandBuffer->End()) {
        return {LAB_ERROR_INVALID_STATE, "Failed to end command buffer"};
    }
    m_currentCommandBuffer.reset();
    return {LAB_ERROR_NONE, nullptr};
}

void MetalBackend::DestroyTexture(Texture* texture) {
    for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
        if (it->get() == texture) {
            m_textures.erase(it);
            break;
        }
    }
}

void MetalBackend::DestroyRenderTarget(RenderTarget* target) {
    for (auto it = m_renderTargets.begin(); it != m_renderTargets.end(); ++it) {
        if (it->get() == target) {
            m_renderTargets.erase(it);
            break;
        }
    }
}

size_t MetalBackend::GetTextureMemoryUsage() const {
    // TODO: Implement texture memory tracking
    return 0;
}

size_t MetalBackend::GetTotalMemoryUsage() const {
    return GetTextureMemoryUsage();
}

bool MetalBackend::SupportsTextureFormat(TextureFormat format) const {
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

bool MetalBackend::SupportsBlendMode(BlendMode mode) const {
    return true;  // Metal supports all blend modes
}

uint32_t MetalBackend::GetMaxTextureSize() const {
    return 16384;  // Metal supports up to 16K textures
}

} // namespace metal
} // namespace labfont
