#ifndef LABFONT_WGPU_DEVICE_H
#define LABFONT_WGPU_DEVICE_H

#include <webgpu/webgpu_cpp.h>

namespace labfont {

struct WGPUDeviceImpl;
typedef WGPUDeviceImpl* WGPUDevice;

class WebGPUDevice {
public:
    WGPUDevice() = default;
    ~WGPUDevice() {
        if (device) wgpuDeviceRelease(device);
        if (queue) wgpuQueueRelease(queue);
    }

    WGPUDevice GetDevice() const { return device; }
    WGPUQueue GetQueue() const { return queue; }

    WGPUDevice device = nullptr;
    WGPUQueue queue = nullptr;
};

} // namespace labfont

#endif // LABFONT_WGPU_DEVICE_H
