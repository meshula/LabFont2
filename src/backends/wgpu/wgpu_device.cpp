#include "wgpu_device.h"
#include "wgpu_types.h"

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
  #include <emscripten/emscripten.h>
#endif

namespace labfont {

WebGPUDevice::~WebGPUDevice() {
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
    if (device) {
        wgpuDeviceRelease(device);
    }
    if (queue) {
        wgpuQueueRelease(queue);
    }
#endif
}

} // namespace labfont
