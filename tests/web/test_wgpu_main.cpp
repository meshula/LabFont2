#include <GLFW/glfw3.h>
#include <webgpu/webgpu_cpp.h>
#include <emscripten/emscripten.h>
#include <iostream>

#include "labfont/labfont.h"
#include "labfont/labfont_types.h"
#include "../utils/test_patterns.h"

// Global state
static wgpu::Instance instance;
static wgpu::Adapter adapter;
static wgpu::Device device;
static wgpu::Surface surface;
static wgpu::TextureFormat format;
static const uint32_t kWidth = 512;
static const uint32_t kHeight = 512;

// Test state
static lab_context ctx = nullptr;
static bool tests_completed = false;
static int test_result = 0;

void ConfigureSurface() {
    wgpu::SurfaceCapabilities capabilities;
    surface.GetCapabilities(adapter, &capabilities);
    format = capabilities.formats[0];

    wgpu::SurfaceConfiguration config{
        .device = device,
        .format = format,
        .width = kWidth,
        .height = kHeight
    };
    surface.Configure(&config);
}

void GetAdapter(void (*callback)(wgpu::Adapter)) {
    instance.RequestAdapter(
        nullptr,
        [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter,
           const char* message, void* userdata) {
            if (message) {
                printf("RequestAdapter: %s\n", message);
            }
            if (status != WGPURequestAdapterStatus_Success) {
                exit(0);
            }
            wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
            reinterpret_cast<void (*)(wgpu::Adapter)>(userdata)(adapter);
        }, 
        reinterpret_cast<void*>(callback));
}

void GetDevice(void (*callback)(wgpu::Device)) {
    adapter.RequestDevice(
        nullptr,
        [](WGPURequestDeviceStatus status, WGPUDevice cDevice,
            const char* message, void* userdata) {
            if (message) {
                printf("RequestDevice: %s\n", message);
            }
            wgpu::Device device = wgpu::Device::Acquire(cDevice);
            device.SetUncapturedErrorCallback(
                [](WGPUErrorType type, const char* message, void* userdata) {
                    std::cout << "Error: " << type << " - message: " << message;
                },
                nullptr);
            reinterpret_cast<void (*)(wgpu::Device)>(userdata)(device);
        }, 
        reinterpret_cast<void*>(callback));
}

void RunTests() {
    // Initialize LabFont context with WebGPU backend
    lab_backend_desc backend_desc = {
        .type = LAB_BACKEND_WGPU,
        .width = kWidth,
        .height = kHeight,
        .native_window = nullptr
    };
    
    lab_result result = lab_create_context(&backend_desc, &ctx);
    if (result != LAB_RESULT_OK) {
        printf("Failed to create context: %s\n", result.message);
        test_result = 1;
        tests_completed = true;
        return;
    }

    // Run basic drawing tests
    result = test_draw_triangle(ctx);
    if (result != LAB_RESULT_OK) {
        printf("Triangle test failed: %s\n", result.message);
        test_result = 1;
        tests_completed = true;
        return;
    }

    // More tests can be added here...

    printf("All tests passed!\n");
    test_result = 0;
    tests_completed = true;
}

void Render() {
    if (!tests_completed) {
        RunTests();
    }
}

void Start() {
    if (!glfwInit()) {
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(kWidth, kHeight, "LabFont WebGPU Tests", nullptr, nullptr);

    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
    canvasDesc.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDesc{.nextInChain = &canvasDesc};
    surface = instance.CreateSurface(&surfaceDesc);

    ConfigureSurface();

    emscripten_set_main_loop(Render, 0, false);
}

int main() {
    instance = wgpu::CreateInstance();
    GetAdapter([](wgpu::Adapter a) {
        adapter = a;
        GetDevice([](wgpu::Device d) {
            device = d;
            Start();
        });
    });
    return 0;
}
