#ifndef LABFONT_WGPU_DEVICE_H
#define LABFONT_WGPU_DEVICE_H

// Forward declarations for WebGPU types to avoid include errors during development
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
  #include <webgpu/webgpu.h>
  #include <emscripten/emscripten.h>
#else
  // Forward declarations for development without Emscripten
  typedef struct WGPUDeviceImpl* WGPUDevice;
  typedef struct WGPUQueueImpl* WGPUQueue;
  
  // Dummy declaration of the Emscripten function
  extern "C" {
    WGPUDevice emscripten_webgpu_get_device();
  }
#endif

namespace labfont {

class WebGPUDevice {
public:
    WebGPUDevice() = default;
    ~WebGPUDevice() {
        // In a real implementation with WebGPU headers:
        // if (device) wgpuDeviceRelease(device);
        // if (queue) wgpuQueueRelease(queue);
    }

    WGPUDevice GetDevice() const { return device; }
    WGPUQueue GetQueue() const { return queue; }

    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;
};

} // namespace labfont

#endif // LABFONT_WGPU_DEVICE_H
