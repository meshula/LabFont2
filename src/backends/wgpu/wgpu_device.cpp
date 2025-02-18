#include "wgpu_backend.h"
#include "wgpu_types.h"

namespace labfont {
namespace wgpu {

WGPUDevice::~WGPUDevice() {
    if (bindGroupLayout) {
        wgpuBindGroupLayoutRelease(bindGroupLayout);
    }
    if (linePipeline) {
        wgpuRenderPipelineRelease(linePipeline);
    }
    if (trianglePipeline) {
        wgpuRenderPipelineRelease(trianglePipeline);
    }
    if (shaderModule) {
        wgpuShaderModuleRelease(shaderModule);
    }
    if (queue) {
        wgpuQueueRelease(queue);
    }
    if (device) {
        wgpuDeviceRelease(device);
    }
}

} // namespace wgpu
} // namespace labfont
