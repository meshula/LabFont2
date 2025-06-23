// Enhanced coordinate system example demonstrating proper transformations

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <labfont/labfont.h>
#include <iostream>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__)
    #import <Metal/Metal.h>
    #import <QuartzCore/CAMetalLayer.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
#elif defined(__EMSCRIPTEN__)
    #include <emscripten.h>
    #include <emscripten/html5.h>
#endif

// Window dimensions
const uint32_t kWidth = 512;
const uint32_t kHeight = 512;

// Global variables
lab_context context = nullptr;
lab_render_target render_target = nullptr;
GLFWwindow* window = nullptr;
static uint8_t* framebuffer = nullptr;
static size_t framebuffer_size = 0;

#ifdef __APPLE__
static id<MTLDevice> gpu = nil;
static id<MTLCommandQueue> queue = nil;
static id<MTLRenderPipelineState> pipelineState = nil;
static CAMetalLayer* swapchain = nil;
static id<MTLBuffer> fsQuadBuffer = nil;
id<MTLTexture> framebuffer_texture = nil;
int fb_texture_width = 0;
int fb_texture_height = 0;

@interface MetalView : NSView
@property (nonatomic, strong) CAMetalLayer *metalLayer;
@end

@implementation MetalView
+ (Class)layerClass {
    return [CAMetalLayer class];
}
@end
#endif

// Coordinate system
lab_coordinate_system coord_system;

static void WindowShouldClose(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

static void WindowResize(GLFWwindow *window, int width, int height) {
    lab_resize_render_target(context, render_target, width, height);
    
#ifdef __APPLE__x
    id nswin = (id)glfwGetCocoaWindow(window);
    MetalView *view = (MetalView *)[nswin contentView];
    view.metalLayer.drawableSize = CGSizeMake(width, height);
#endif
}


bool InitWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(kWidth, kHeight, "LabFont Coordinate System Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, WindowShouldClose);
    glfwSetWindowSizeCallback(window, WindowResize);
    
#ifdef __APPLE__
    gpu = MTLCreateSystemDefaultDevice();
    id nswin = (id)glfwGetCocoaWindow(window);
    NSView* contentView = [nswin contentView];
    contentView.wantsLayer = YES;
    swapchain = [CAMetalLayer layer];
    swapchain.device = gpu;
    swapchain.pixelFormat = MTLPixelFormatBGRA8Unorm;
    swapchain.opaque = YES;
    swapchain.drawableSize = CGSizeMake(kWidth, kHeight);
    contentView.layer = swapchain;
    
    // Define a full-screen quad (clip space coordinates)
    static const float fullscreenQuad[] = {
        // Position       // Texture Coordinates
        -1, -1,  0, 1,   0, 0,  // Bottom-left
         1, -1,  0, 1,   1, 0,  // Bottom-right
        -1,  1,  0, 1,   0, 1,  // Top-left
        -1,  1,  0, 1,   0, 1,  // Top-left
         1, -1,  0, 1,   1, 0,  // Bottom-right
         1,  1,  0, 1,   1, 1   // Top-right
    };

    // Create vertex buffer
    fsQuadBuffer = [gpu newBufferWithBytes:fullscreenQuad
                                       length:sizeof(fullscreenQuad)
                                      options:MTLResourceStorageModeShared];

    
    MTLVertexDescriptor* vertexDescriptor = [MTLVertexDescriptor new];
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat4;  // Position attribute
    vertexDescriptor.attributes[0].offset = 0;
    vertexDescriptor.attributes[0].bufferIndex = 0;

    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;  // Texture coordinates
    vertexDescriptor.attributes[1].offset = sizeof(float) * 4;  // Skip the 4 floats for position
    vertexDescriptor.attributes[1].bufferIndex = 0;

    vertexDescriptor.layouts[0].stride = sizeof(float) * 6;  // Position + TexCoords (4 + 2)
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    vertexDescriptor.layouts[0].stepRate = 1;
    
    queue = [gpu newCommandQueue];
    
    NSString* vertexShaderSrc = @R"metal(
    #include <metal_stdlib>
    using namespace metal;

    struct VertexIn {
        float4 position [[attribute(0)]];
        float2 texCoord [[attribute(1)]];
    };

    struct VertexOut {
        float4 position [[position]];
        float2 texCoord;
    };

    vertex VertexOut vertex_main(VertexIn in [[stage_in]]) {
        VertexOut out;
        out.position = in.position;
        out.texCoord = in.texCoord;  // Pass texture coordinates to fragment shader
        return out;
    }
    
    struct FragmentIn {
        float2 texCoord;
    };

    fragment float4 fragment_main(FragmentIn in [[stage_in]], 
                                  texture2d<float> texture [[texture(0)]]) {
        constexpr sampler s; // Default sampler

        // Sample the texture at the given texture coordinate
        return texture.sample(s, in.texCoord);
    }
    )metal";
        
    NSError* error = nil;
    id<MTLLibrary> library = [gpu newLibraryWithSource:vertexShaderSrc options:nil error:&error];
    
    if (!library) {
        NSLog(@"Failed to compile vertex shader: %@", error);
    }
    
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];
    
    MTLRenderPipelineDescriptor *pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineDesc.vertexFunction = vertexFunction;
    pipelineDesc.fragmentFunction = fragmentFunction;
    pipelineDesc.vertexDescriptor = vertexDescriptor;
    pipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    pipelineState = [gpu newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (!pipelineState) {
        NSLog(@"Failed to create pipeline state: %@", error);
        return false;
    }
#endif
    return true;
}


void BlitToWindow(int width, int height, uint8_t* framebuffer, size_t framebuffer_size) {
#ifdef _WIN32
    HDC hdc = GetDC(glfwGetWin32Window(window));
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height, framebuffer, &bmi, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(glfwGetWin32Window(window), hdc);
#elif defined(__APPLE__)
    // Check if framebuffer needs to be recreated
    if (width != fb_texture_width || height != fb_texture_height) {
        fb_texture_width = width;
        fb_texture_height = height;
        
        // Create or recreate the framebuffer texture
        MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                                   width:width
                                                                                                  height:height
                                                                                               mipmapped:NO];
        framebuffer_texture = [gpu newTextureWithDescriptor:textureDescriptor];
    }
    if (!framebuffer_texture) {
        return;
    }
    
    // Upload framebuffer data (assuming `framebuffer` holds pixel data)
    [framebuffer_texture replaceRegion:MTLRegionMake2D(0, 0, width, height)
                            mipmapLevel:0
                              withBytes:framebuffer
                            bytesPerRow:width * 4]; // assuming 4 bytes per pixel (RGBA)


    // Use a static clear color that pulses
    static MTLClearColor color = MTLClearColorMake(0, 0, 0, 1);
    color.green = (color.green > 1.0) ? 0 : color.green + 0.01;

    // Get the drawable surface for the swapchain
    id<CAMetalDrawable> surface = [swapchain nextDrawable];

    // Create a render pass descriptor
    MTLRenderPassDescriptor *pass = [MTLRenderPassDescriptor renderPassDescriptor];
    pass.colorAttachments[0].clearColor = color;
    pass.colorAttachments[0].loadAction  = MTLLoadActionClear;
    pass.colorAttachments[0].storeAction = MTLStoreActionStore;
    pass.colorAttachments[0].texture = surface.texture;

    // Create a command buffer and render command encoder
    id<MTLCommandBuffer> buffer = [queue commandBuffer];
    id<MTLRenderCommandEncoder> encoder = [buffer renderCommandEncoderWithDescriptor:pass];

    // Set up the pipeline state and the vertex buffer
    [encoder setRenderPipelineState:pipelineState];
    [encoder setVertexBuffer:fsQuadBuffer offset:0 atIndex:0];
    
    // If using a texture, set it as a fragment shader input (assuming the texture is passed to the pipeline)
    [encoder setFragmentTexture:framebuffer_texture atIndex:0];

    // Draw the quad
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];

    // End encoding and commit the command buffer
    [encoder endEncoding];
    [buffer presentDrawable:surface];
    [buffer commit];
#elif defined(__linux__)
    Display* display = glfwGetX11Display();
    Window win = glfwGetX11Window(window);
    GC gc = XCreateGC(display, win, 0, 0);
    XImage* image = XCreateImage(display, DefaultVisual(display, 0), 24, ZPixmap, 0, (char*)framebuffer, width, height, 32, 0);
    XPutImage(display, win, gc, image, 0, 0, 0, 0, width, height);
    XDestroyImage(image);
    XFreeGC(display, gc);
#elif defined(__EMSCRIPTEN__)
    EM_ASM({
        var canvas = document.getElementsByTagName('canvas')[0].getContext('2d');
        var imageData = canvas.createImageData($0, $1);
        var buffer = new Uint8Array(Module.HEAPU8.buffer, $2, $0 * $1 * 4);
        imageData.data.set(buffer);
        canvas.putImageData(imageData, 0, 0);
    }, width, height, framebuffer.data());
#endif
}

bool InitLabFont() {
    // Create a backend descriptor
    lab_backend_desc backend_desc = {
        .type = LAB_BACKEND_CPU, // Use CPU backend to test coordinate system
        .width = kWidth,
        .height = kHeight,
        .native_window = window
    };

    // Create the context
    lab_result result = lab_create_context(&backend_desc, &context);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to create LabFont context: " << lab_get_error_string(result) << std::endl;
        return false;
    }

    // Create a render target
    lab_render_target_desc rt_desc = {
        .width = kWidth,
        .height = kHeight,
        .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
        .hasDepth = true
    };
    
    result = lab_create_render_target(context, &rt_desc, &render_target);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to create render target: " << lab_get_error_string(result) << std::endl;
        return false;
    }
    
    // Set the render target
    result = lab_set_render_target(context, render_target);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to set render target: " << lab_get_error_string(result) << std::endl;
        return false;
    }

    // Get the default coordinate system
    result = lab_get_coordinate_system(context, &coord_system);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to get coordinate system: " << lab_get_error_string(result) << std::endl;
        return false;
    }

    std::cout << "Coordinate system initialized successfully!" << std::endl;
    std::cout << "Device space: (" << coord_system.desc.device_origin[0] << ", " << coord_system.desc.device_origin[1] 
              << ") size (" << coord_system.desc.device_size[0] << ", " << coord_system.desc.device_size[1] << ")" << std::endl;
    std::cout << "Local space: (" << coord_system.desc.local_origin[0] << ", " << coord_system.desc.local_origin[1] 
              << ") size (" << coord_system.desc.local_size[0] << ", " << coord_system.desc.local_size[1] << ")" << std::endl;

    return true;
}

// Global persistent vertex data (pre-transformed)
static std::vector<lab_vertex_2TC> g_redTriangleVertices;
static std::vector<lab_vertex_2TC> g_greenTriangleVertices;
static bool g_verticesInitialized = false;

// Initialize pre-transformed vertices (call once after coordinate system setup)
void InitializeTransformedVertices() {
    if (g_verticesInitialized) return;
    
    // Create local coordinate vertices
    std::vector<lab_vertex_2TC> redLocal = {
        {.position = {0.0f, 0.5f}, .texcoord = {0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f, 1.0f}},
        {.position = {-0.5f, -0.5f}, .texcoord = {0.0f, 1.0f}, .color = {1.0f, 0.0f, 0.0f, 1.0f}},
        {.position = {0.5f, -0.5f}, .texcoord = {1.0f, 1.0f}, .color = {1.0f, 0.0f, 0.0f, 1.0f}}
    };
    
    std::vector<lab_vertex_2TC> greenLocal = {
        {.position = {0.2f, 0.8f}, .texcoord = {0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f, 1.0f}},
        {.position = {-0.2f, 0.2f}, .texcoord = {0.0f, 1.0f}, .color = {0.0f, 1.0f, 0.0f, 1.0f}},
        {.position = {0.6f, 0.2f}, .texcoord = {1.0f, 1.0f}, .color = {0.0f, 1.0f, 0.0f, 1.0f}}
    };
    
    // Pre-transform vertices once
    g_redTriangleVertices.resize(redLocal.size());
    g_greenTriangleVertices.resize(greenLocal.size());
    
    for (size_t i = 0; i < redLocal.size(); ++i) {
        lab_result result = lab_transform_vertex(&coord_system, LAB_COORD_LOCAL, LAB_COORD_NORMALIZED,
                                               &redLocal[i], &g_redTriangleVertices[i]);
        if (result != LAB_RESULT_OK) {
            std::cerr << "Failed to transform red vertex " << i << ": " << lab_get_error_string(result) << std::endl;
            return;
        }
    }
    
    for (size_t i = 0; i < greenLocal.size(); ++i) {
        lab_result result = lab_transform_vertex(&coord_system, LAB_COORD_LOCAL, LAB_COORD_NORMALIZED,
                                               &greenLocal[i], &g_greenTriangleVertices[i]);
        if (result != LAB_RESULT_OK) {
            std::cerr << "Failed to transform green vertex " << i << ": " << lab_get_error_string(result) << std::endl;
            return;
        }
    }
    
    g_verticesInitialized = true;
    std::cout << "Vertices pre-transformed successfully!" << std::endl;
}

lab_result Blit() {
    // Get the framebuffer data from the render target
    lab_render_target_desc rt_desc;
    lab_result result = lab_get_render_target_data(context, render_target, &rt_desc, &framebuffer, &framebuffer_size);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to get render target data: " << lab_get_error_string(result);
        return result;
    }
    
    // Blit the framebuffer to the window
    BlitToWindow(rt_desc.width, rt_desc.height, framebuffer, framebuffer_size);
    
    return LAB_RESULT_OK;
}

lab_result Render() {
    // Begin the frame
    lab_result result = lab_begin_frame(context);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to begin frame: " << lab_get_error_string(result) << std::endl;
        return result;
    }
    
    // Create clear command (black background)
    lab_draw_command clear_cmd = {
        .type = LAB_DRAW_COMMAND_CLEAR,
        .clear = {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        }
    };
    
    // Get current time for demonstration
    static double start_time = glfwGetTime();
    double current_time = glfwGetTime();
    double elapsed = current_time - start_time;
    
    // Alternate between triangles every 2 seconds
    bool use_first_triangle = ((int)(elapsed * 0.5)) % 2 == 0;
    
    std::vector<lab_draw_command> commands;
    commands.push_back(clear_cmd);
    
    if (use_first_triangle) {
        std::cout << "\rRendering red triangle (pre-transformed, persistent vertices)" << std::flush;
        
        // Use pre-transformed persistent vertices - no scope issues!
        lab_draw_command triangle_cmd = {
            .type = LAB_DRAW_COMMAND_TRIANGLES,
            .triangles = {
                .vertices = g_redTriangleVertices.data(),
                .vertexCount = static_cast<uint32_t>(g_redTriangleVertices.size())
            }
        };
        commands.push_back(triangle_cmd);
    } else {
        std::cout << "\rRendering green triangle (pre-transformed, persistent vertices)" << std::flush;
        
        // Use pre-transformed persistent vertices - no scope issues!
        lab_draw_command triangle_cmd = {
            .type = LAB_DRAW_COMMAND_TRIANGLES,
            .triangles = {
                .vertices = g_greenTriangleVertices.data(),
                .vertexCount = static_cast<uint32_t>(g_greenTriangleVertices.size())
            }
        };
        commands.push_back(triangle_cmd);
    }
    
    // Submit all commands
    result = lab_submit_commands(context, commands.data(), static_cast<uint32_t>(commands.size()));
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to submit commands: " << lab_get_error_string(result) << std::endl;
        return result;
    }
    
    // End the frame
    result = lab_end_frame(context);
    if (result != LAB_RESULT_OK) {
        std::cerr << "Failed to end frame: " << lab_get_error_string(result) << std::endl;
        return result;
    }
    
    return LAB_RESULT_OK;
}

lab_result SaveFrame() {
    // Save the render target to a file (only once)
    static bool saved = false;
    if (!saved) {
        lab_result result = lab_save_render_target(context, render_target, "coordinate_system_test.png");
        if (result != LAB_RESULT_OK) {
            std::cerr << "Failed to save render target: " << lab_get_error_string(result) << std::endl;
            return result;
        } else {
            std::cout << "\nCoordinate system test saved to coordinate_system_test.png" << std::endl;
            saved = true;
        }
    }
    return LAB_RESULT_OK;
}

void Cleanup() {
    if (framebuffer) {
        lab_free(framebuffer);
        framebuffer = nullptr;
        framebuffer_size = 0;
    }
    
    if (render_target && context) {
        lab_destroy_render_target(context, render_target);
        render_target = nullptr;
    }
    
    if (context) {
        lab_destroy_context(context);
        context = nullptr;
    }

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

int main() {
    std::cout << "LabFont2 Coordinate System Test" << std::endl;
    std::cout << "=================================" << std::endl;
    
    // Initialize window
    if (!InitWindow()) {
        return 1;
    }

    // Initialize LabFont
    if (!InitLabFont()) {
        Cleanup();
        return 1;
    }

    std::cout << "\nStarting render loop. Press ESC to exit." << std::endl;
    std::cout << "Red triangle: Local coordinates transformed to normalized coordinates" << std::endl;
    std::cout << "Green triangle: Demonstrates coordinate system transformation" << std::endl;
    
    // Run a few frames to demonstrate coordinate system
    int frameCount = 0;
    const int maxFrames = 10; // Limit frames for testing
    
    InitializeTransformedVertices();
    
    while (!glfwWindowShouldClose(window) && frameCount < maxFrames) {
        glfwPollEvents();
        
        if (Render() != LAB_RESULT_OK) {
            break;
        }
        
        // Blit the rendered frame to the window
        if (Blit() != LAB_RESULT_OK) {
            std::cerr << "Failed to blit frame to window" << std::endl;
            break;
        }
        
        // Save first frame
        if (frameCount == 0) {
            SaveFrame();
        }
        
        frameCount++;
        
        // Small delay to see the alternating triangles
        glfwWaitEventsTimeout(0.5);
    }

    std::cout << "\n\nCoordinate system test completed successfully!" << std::endl;
    std::cout << "Rendered " << frameCount << " frames with proper coordinate transformations." << std::endl;
    
    // Cleanup
    Cleanup();
    return 0;
}
