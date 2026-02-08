#import "metal_backend.h"
#import "metal_command_buffer.h"
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <iostream>
#include <sstream>
#include <sys/sysctl.h>

namespace labfont {
namespace metal {

namespace {
    MTLPixelFormat TextureFormatToMTLFormat(lab_texture_format format) {
        switch (format) {
            case LAB_TEXTURE_FORMAT_R8_UNORM:        return MTLPixelFormatR8Unorm;
            case LAB_TEXTURE_FORMAT_RGBA8_UNORM:     return MTLPixelFormatRGBA8Unorm;
            case LAB_TEXTURE_FORMAT_BGRA8_UNORM_SRGB: return MTLPixelFormatBGRA8Unorm_sRGB;
            case LAB_TEXTURE_FORMAT_R16F:            return MTLPixelFormatR16Float;
            case LAB_TEXTURE_FORMAT_RGBA16F:         return MTLPixelFormatRGBA16Float;
            case LAB_TEXTURE_FORMAT_R32F:            return MTLPixelFormatR32Float;
            case LAB_TEXTURE_FORMAT_D32F:            return MTLPixelFormatDepth32Float;
            default:
                std::cerr << "Unsupported texture format" << std::endl;
                return MTLPixelFormatInvalid;
        }
    }
} // anon

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
    textureDesc.textureType = MTLTextureType2D;
    textureDesc.width = desc.width;
    textureDesc.height = desc.height;
    textureDesc.pixelFormat = TextureFormatToMTLFormat(m_format);

    if (desc.renderTarget) {
        textureDesc.usage |= MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        textureDesc.storageMode = MTLStorageModeShared;
    }
    else {
        textureDesc.usage = MTLTextureUsageShaderRead;
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
            .format = LAB_TEXTURE_FORMAT_D32F,
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
    m_renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0); // Black

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

void TestMetalOffscreenRendering() {
    // 1. Create Metal device
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device) {
        std::cerr << "Error: No Metal device found!\n";
        return;
    }

    // 2. Create command queue
    id<MTLCommandQueue> commandQueue = [device newCommandQueue];
    if (!commandQueue) {
        std::cerr << "Error: Failed to create command queue!\n";
        return;
    }

    // 3. Create command buffer
    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
    if (!commandBuffer) {
        std::cerr << "Error: Failed to create command buffer!\n";
        return;
    }

    // 4. Create an offscreen texture (color attachment)
    MTLTextureDescriptor* colorDesc = [[MTLTextureDescriptor alloc] init];
    colorDesc.textureType = MTLTextureType2D;
    colorDesc.pixelFormat = MTLPixelFormatRGBA8Unorm;
    colorDesc.width = 512;
    colorDesc.height = 512;
    colorDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    colorDesc.storageMode = MTLStorageModeShared;

    id<MTLTexture> colorTexture = [device newTextureWithDescriptor:colorDesc];
    if (!colorTexture) {
        std::cerr << "Error: Failed to create offscreen color texture!\n";
        return;
    }

    // 5. Create render pass descriptor
    MTLRenderPassDescriptor* renderPassDesc = [[MTLRenderPassDescriptor alloc] init];
    renderPassDesc.colorAttachments[0].texture = colorTexture;
    renderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
    renderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
    renderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0); // Black

    // 6. Create render command encoder
    id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];
    if (!renderEncoder) {
        std::cerr << "Error: Failed to create render command encoder!\n";
        return;
    }

    std::cout << "✅ Success: Render encoder created without crashing!\n";

    // 7. Finish encoding
    [renderEncoder endEncoding];

    // 8. Commit and wait
    [commandBuffer commit];
    [commandBuffer waitUntilCompleted];
}

MetalDevice::MetalDevice()
    : m_device(nil)
    , m_commandQueue(nil)
    , m_shaderLibrary(nil)
    , m_trianglePipeline(nil)
    , m_texturedTrianglePipeline(nil)
    , m_linePipeline(nil)
    , m_depthState(nil)
{
    //TestMetalOffscreenRendering();
}

MetalDevice::~MetalDevice() {
    if (m_depthState) [m_depthState release];
    if (m_linePipeline) [m_linePipeline release];
    if (m_trianglePipeline) [m_trianglePipeline release];
    if (m_texturedTrianglePipeline) [m_texturedTrianglePipeline release];
    if (m_shaderLibrary) [m_shaderLibrary release];
    if (m_commandQueue) [m_commandQueue release];
    if (m_device) [m_device release];
}

bool MetalDevice::Initialize() {
    // Create Metal device
    m_device = MTLCreateSystemDefaultDevice();
    if (!m_device) {
        std::cerr << "Error: No Metal device found!\n";
        std::cerr << "System information:\n";
        
        // Get macOS version
        NSProcessInfo* processInfo = [NSProcessInfo processInfo];
        NSOperatingSystemVersion osVersion = [processInfo operatingSystemVersion];
        std::cerr << "  - macOS version: " << osVersion.majorVersion << "." 
                  << osVersion.minorVersion << "." << osVersion.patchVersion << "\n";
        
        // Check if Metal is supported on this system
        if (@available(macOS 10.11, *)) {
            std::cerr << "  - Metal API should be available (macOS 10.11+)\n";
        } else {
            std::cerr << "  - Metal API may not be available (requires macOS 10.11+)\n";
        }
        
        // Check if we're running in a virtual machine
        size_t size;
        sysctlbyname("hw.model", NULL, &size, NULL, 0);
        char* model = (char*)malloc(size);
        sysctlbyname("hw.model", model, &size, NULL, 0);
        std::string modelStr(model);
        free(model);
        
        bool isVirtualMachine = (modelStr.find("VMware") != std::string::npos || 
                                modelStr.find("Virtual") != std::string::npos ||
                                modelStr.find("QEMU") != std::string::npos ||
                                modelStr.find("Parallels") != std::string::npos);
        
        std::cerr << "  - Hardware model: " << modelStr << "\n";
        if (isVirtualMachine) {
            std::cerr << "  - Running in a virtual machine. Metal may have limited or no support.\n";
        }
        
        return false;
    }
    
    // Log device information
    std::cerr << "Metal device information:\n";
    std::cerr << "  - Device name: " << [[m_device name] UTF8String] << "\n";
    std::cerr << "  - Registry ID: " << [m_device registryID] << "\n";
    
    if (@available(macOS 10.15, *)) {
        std::cerr << "  - Is low power: " << ([m_device isLowPower] ? "Yes" : "No") << "\n";
        std::cerr << "  - Is removable: " << ([m_device isRemovable] ? "Yes" : "No") << "\n";
    }
    
    if (@available(macOS 11.0, *)) {
        std::cerr << "  - Has unified memory: " << ([m_device hasUnifiedMemory] ? "Yes" : "No") << "\n";
    }
    
    // Create command queue
    m_commandQueue = [m_device newCommandQueue];
    if (!m_commandQueue) {
        std::cerr << "Error: Failed to create command queue!\n";
        std::cerr << "  - Device: " << [[m_device name] UTF8String] << "\n";
        
        // Check if there are any resource limits
        if (@available(macOS 11.0, *)) {
            std::cerr << "  - Max buffer length: " << [m_device maxBufferLength] << " bytes\n";
            std::cerr << "  - Max threadgroup memory length: " << [m_device maxThreadgroupMemoryLength] << " bytes\n";
        }
        
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
    // First, try to find the shader library in the app bundle
    NSString* libraryPath = [[NSBundle mainBundle] pathForResource:@"primitives" ofType:@"metallib"];
    if (!libraryPath) {
        // If not found in the bundle, check the executable directory
        NSString* executablePath = [[NSBundle mainBundle] executablePath];
        NSString* executableDir = [executablePath stringByDeletingLastPathComponent];
        libraryPath = [executableDir stringByAppendingPathComponent:@"primitives.metallib"];
        
        if (![[NSFileManager defaultManager] fileExistsAtPath:libraryPath]) {
            // Log all the places we looked for the shader library
            std::cerr << "Error: Metal shader library 'primitives.metallib' not found." << std::endl;
            std::cerr << "  - Searched in app bundle: " << [[[NSBundle mainBundle] bundlePath] UTF8String] << std::endl;
            std::cerr << "  - Searched in executable directory: " << [executableDir UTF8String] << std::endl;
            
            // Also check the current working directory
            NSString* currentDir = [[NSFileManager defaultManager] currentDirectoryPath];
            NSString* currentDirLibPath = [currentDir stringByAppendingPathComponent:@"primitives.metallib"];
            std::cerr << "  - Searched in current directory: " << [currentDir UTF8String] << std::endl;
            
            // Check if the shader source file exists
            NSString* shaderSourcePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"src/backends/metal/shaders/primitives.metal"];
            if ([[NSFileManager defaultManager] fileExistsAtPath:shaderSourcePath]) {
                std::cerr << "  - Metal shader source file exists at: " << [shaderSourcePath UTF8String] << std::endl;
                std::cerr << "  - Try running the shader compilation script: src/backends/metal/shaders/compile_shaders.sh" << std::endl;
            }
            
            return false;
        }
    }
    
    NSURL* libraryURL = [NSURL fileURLWithPath:libraryPath];
    NSError* error = nil;
    m_shaderLibrary = [m_device newLibraryWithURL:libraryURL error:&error];
    if (!m_shaderLibrary) {
        std::cerr << "Error: Failed to load Metal shader library from: " << [libraryPath UTF8String] << std::endl;
        if (error) {
            std::cerr << "  - Error details: " << [[error localizedDescription] UTF8String] << std::endl;
        }
        return false;
    }
    
    std::cout << "Successfully loaded Metal shader library from: " << [libraryPath UTF8String] << std::endl;
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
    id<MTLFunction> fragmentFunc = [m_shaderLibrary newFunctionWithName:@"fragment_color"];
    id<MTLFunction> texturedFragmentFunc = [m_shaderLibrary newFunctionWithName:@"fragment_texture"];

    if (!vertexFunc) {
        std::cerr << "Error: Failed to find vertex shader function 'vertex_main' in shader library\n";
        [vertexDesc release];
        [pipelineDesc release];
        return false;
    }
    if (!fragmentFunc) {
        std::cerr << "Error: Failed to find fragment shader function 'fragment_color' in shader library\n";
        [vertexFunc release];
        [vertexDesc release];
        [pipelineDesc release];
        return false;
    }
    if (!texturedFragmentFunc) {
        std::cerr << "Error: Failed to find fragment shader function 'fragment_texture' in shader library\n";
        [vertexDesc release];
        [pipelineDesc release];
        return false;
    }
    
    pipelineDesc.vertexFunction = vertexFunc;
    pipelineDesc.fragmentFunction = fragmentFunc;
    
    NSError* error = nil;
    m_trianglePipeline = [m_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];

    pipelineDesc.fragmentFunction = texturedFragmentFunc;
    m_texturedTrianglePipeline = [m_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];

    [vertexFunc release];
    [fragmentFunc release];
    [texturedFragmentFunc release];
    
    if (!m_trianglePipeline) {
        std::cerr << "Error: Failed to create triangle pipeline state\n";
        if (error) {
            std::cerr << "  - Error details: " << [[error localizedDescription] UTF8String] << "\n";
        }
        [vertexDesc release];
        [pipelineDesc release];
        return false;
    }
    
    // Line pipeline
    fragmentFunc = [m_shaderLibrary newFunctionWithName:@"fragment_line"];
    if (!fragmentFunc) {
        std::cerr << "Error: Failed to find fragment shader function 'fragment_line' in shader library\n";
        [vertexDesc release];
        [pipelineDesc release];
        return false;
    }
    
    pipelineDesc.fragmentFunction = fragmentFunc;
    
    error = nil;
    m_linePipeline = [m_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    
    [fragmentFunc release];
    [vertexDesc release];
    [pipelineDesc release];
    
    if (!m_linePipeline) {
        std::cerr << "Error: Failed to create line pipeline state\n";
        if (error) {
            std::cerr << "  - Error details: " << [[error localizedDescription] UTF8String] << "\n";
        }
        return false;
    }
    
    // Create depth state
    MTLDepthStencilDescriptor* depthDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
    depthDesc.depthWriteEnabled = YES;
    
    m_depthState = [m_device newDepthStencilStateWithDescriptor:depthDesc];
    [depthDesc release];
    
    if (!m_depthState) {
        std::cerr << "Error: Failed to create depth stencil state\n";
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
    
    // Capture stderr output to get detailed error messages
    std::stringstream errorStream;
    std::streambuf* oldCerr = std::cerr.rdbuf(errorStream.rdbuf());
    
    std::cerr << "=== Metal Initialization Begin ===\n";
    
    // Log system information
    NSProcessInfo* processInfo = [NSProcessInfo processInfo];
    NSOperatingSystemVersion osVersion = [processInfo operatingSystemVersion];
    std::cerr << "System information:\n";
    std::cerr << "  - macOS version: " << osVersion.majorVersion << "." 
              << osVersion.minorVersion << "." << osVersion.patchVersion << "\n";
    
    // Check physical memory
    std::cerr << "  - Physical memory: " << ([processInfo physicalMemory] / (1024 * 1024)) << " MB\n";
    
    // Check processor count
    std::cerr << "  - Processor count: " << [processInfo processorCount] << "\n";
    std::cerr << "  - Active processor count: " << [processInfo activeProcessorCount] << "\n";
    
    // Check if Metal framework is available
    NSBundle* metalBundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/Metal.framework"];
    if (metalBundle) {
        std::cerr << "  - Metal framework is available\n";
        NSDictionary* infoDictionary = [metalBundle infoDictionary];
        NSString* version = [infoDictionary objectForKey:@"CFBundleShortVersionString"];
        if (version) {
            std::cerr << "  - Metal framework version: " << [version UTF8String] << "\n";
        }
    } else {
        std::cerr << "  - Metal framework is NOT available\n";
    }
    
    // Initialize the Metal device
    bool success = m_device->Initialize();
    
    std::cerr << "=== Metal Initialization End ===\n";
    
    // Restore stderr
    std::cerr.rdbuf(oldCerr);
    
    if (!success) {
        std::string errorMsg = errorStream.str();
        if (errorMsg.find("Metal shader library") != std::string::npos) {
            std::cerr <<  "Failed to initialize Metal device: Metal shader library not found. "
            "Please ensure primitives.metallib is available in the executable directory." << std::endl;
            return LAB_RESULT_SHADER_LIBRARY_INITIALIZATION_FAILED;
        } else {
            return LAB_RESULT_DEVICE_INITIALIZATION_FAILED;
        }
    }
    
    return LAB_RESULT_OK;
}

lab_result MetalBackend::Resize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return LAB_RESULT_OK;
}

lab_result MetalBackend::CreateTexture(const TextureDesc& desc, std::shared_ptr<Texture>& out_texture) {
    auto texture = std::make_shared<MetalTexture>(m_device.get(), desc);
    if (!texture->GetMTLTexture()) {
        return LAB_RESULT_TEXTURE_CREATION_FAILED;
    }
    
    out_texture = texture;
    m_textures.push_back(texture);
    return LAB_RESULT_OK;
}

lab_result MetalBackend::UpdateTexture(Texture* texture, const void* data, size_t size) {
    auto metalTexture = static_cast<MetalTexture*>(texture);
    auto mtlTexture = metalTexture->GetMTLTexture();
    
    [mtlTexture replaceRegion:MTLRegionMake2D(0, 0, texture->GetWidth(), texture->GetHeight())
                mipmapLevel:0
                withBytes:data
                bytesPerRow:texture->GetWidth() * 4];
    
    return LAB_RESULT_OK;
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
    
    return LAB_RESULT_OK;
}

lab_result MetalBackend::CreateRenderTarget(const RenderTargetDesc& desc, std::shared_ptr<RenderTarget>& out_target) {
    auto target = std::make_shared<MetalRenderTarget>(m_device.get(), desc);
    if (!target->GetRenderPassDescriptor()) {
        return LAB_RESULT_RENDERTARGET_INITIALIZATION_FAILED;
    }
    
    out_target = target;
    m_renderTargets.push_back(target);
    return LAB_RESULT_OK;
}

lab_result MetalBackend::SetRenderTarget(RenderTarget* target) {
    m_currentRenderTarget = target;
    return LAB_RESULT_OK;
}

lab_result MetalBackend::BeginFrame() {
    m_currentCommandBuffer = std::make_unique<MetalCommandBuffer>(m_device.get());
    if (!m_currentCommandBuffer->Begin()) {
        return LAB_RESULT_COMMAND_BUFFER_INITIALIZATION_FAILED;
    }
    return LAB_RESULT_OK;
}

lab_result MetalBackend::SubmitCommands(const std::vector<DrawCommand>& commands) {
    if (!m_currentRenderTarget) {
        return LAB_RESULT_INVALID_RENDER_TARGET;
    }
    
    auto metalTarget = static_cast<MetalRenderTarget*>(m_currentRenderTarget);
    auto result = m_currentCommandBuffer->BeginRenderPass(metalTarget);
    if (result != LAB_RESULT_OK) {
        return result;
    }
    
    for (const auto& cmd : commands) {
        switch (cmd.type) {
            case DrawCommandType::Clear: {
                m_currentCommandBuffer->Clear(cmd.clear.color);
                break;
            }
            
            case DrawCommandType::DrawTriangles: {
                const auto& params = cmd.triangles;
                // Convert lab_vertex_2TC to Vertex
                std::vector<Vertex> vertices;
                vertices.reserve(params.vertexCount);
                for (uint32_t i = 0; i < params.vertexCount; ++i) {
                    vertices.push_back(Vertex(params.vertices[i]));
                }
                m_currentCommandBuffer->DrawTriangles(vertices.data(), params.vertexCount);
                break;
            }
            
            case DrawCommandType::DrawLines: {
                const auto& params = cmd.lines;
                // Convert lab_vertex_2TC to Vertex
                std::vector<Vertex> vertices;
                vertices.reserve(params.vertexCount);
                for (uint32_t i = 0; i < params.vertexCount; ++i) {
                    vertices.push_back(Vertex(params.vertices[i]));
                }
                m_currentCommandBuffer->DrawLines(vertices.data(), params.vertexCount, params.lineWidth);
                break;
            }
                
            case DrawCommandType::BindTexture: {
                const auto& params = cmd.bind_texture;
                m_currentCommandBuffer->BindTexture(params.texture);
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
            
            case DrawCommandType::SetViewport:
            case DrawCommandType::SetViewportAPI: {
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
    return LAB_RESULT_OK;
}

lab_result MetalBackend::EndFrame() {
    if (!m_currentCommandBuffer->End()) {
        return LAB_RESULT_INVALID_COMMAND_BUFFER;
    }
    m_currentCommandBuffer.reset();
    return LAB_RESULT_OK;
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

bool MetalBackend::SupportsTextureFormat(lab_texture_format format) const {
    switch (format) {
        case LAB_TEXTURE_FORMAT_R8_UNORM:
        case LAB_TEXTURE_FORMAT_RG8_UNORM:
        case LAB_TEXTURE_FORMAT_RGBA8_UNORM:
        case LAB_TEXTURE_FORMAT_BGRA8_UNORM_SRGB:
        case LAB_TEXTURE_FORMAT_R16F:
        case LAB_TEXTURE_FORMAT_RG16F:
        case LAB_TEXTURE_FORMAT_RGBA16F:
        case LAB_TEXTURE_FORMAT_R32F:
        case LAB_TEXTURE_FORMAT_RG32F:
        case LAB_TEXTURE_FORMAT_RGBA32F:
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
