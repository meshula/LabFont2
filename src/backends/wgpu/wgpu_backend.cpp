#include "wgpu_backend.h"
#include "wgpu_device.h"
#include <emscripten/emscripten.h>

namespace labfont {

WGPUBackend::~WGPUBackend() {
    if (m_shaderModule) wgpuShaderModuleRelease(m_shaderModule);
    if (m_trianglePipeline) wgpuRenderPipelineRelease(m_trianglePipeline);
    if (m_linePipeline) wgpuRenderPipelineRelease(m_linePipeline);
    if (m_bindGroupLayout) wgpuBindGroupLayoutRelease(m_bindGroupLayout);
}

lab_result WGPUBackend::Initialize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    
    // Get WebGPU device from Emscripten
    m_device->device = emscripten_webgpu_get_device();
    if (!m_device->device) {
        return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "Failed to get WebGPU device");
    }
    
    // Get queue
    m_device->queue = wgpuDeviceGetQueue(m_device->device);
    if (!m_device->queue) {
        return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "Failed to get WebGPU queue");
    }
    
    // Create shader module
    const char* shaderSource = R"(
        struct VertexInput {
            @location(0) position: vec2<f32>,
            @location(1) texcoord: vec2<f32>,
            @location(2) color: vec4<f32>,
        };

        struct VertexOutput {
            @builtin(position) position: vec4<f32>,
            @location(0) color: vec4<f32>,
        };

        @vertex
        fn vs_main(input: VertexInput) -> VertexOutput {
            var output: VertexOutput;
            output.position = vec4<f32>(input.position, 0.0, 1.0);
            output.color = input.color;
            return output;
        }

        @fragment
        fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
            return input.color;
        }
    )";
    
    WGPUShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    wgslDesc.code = shaderSource;
    
    WGPUShaderModuleDescriptor shaderDesc = {};
    shaderDesc.nextInChain = reinterpret_cast<WGPUChainedStruct*>(&wgslDesc);
    
    m_shaderModule = wgpuDeviceCreateShaderModule(m_device->GetDevice(), &shaderDesc);
    if (!m_shaderModule) {
        return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "Failed to create shader module");
    }
    
    // Create pipeline layout
    WGPUPipelineLayoutDescriptor layoutDesc = {};
    layoutDesc.bindGroupLayoutCount = 0;
    layoutDesc.bindGroupLayouts = nullptr;
    
    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(m_device->GetDevice(), &layoutDesc);
    if (!pipelineLayout) {
        return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "Failed to create pipeline layout");
    }
    
    // Create render pipeline
    WGPUVertexState vertexState = {};
    vertexState.module = m_shaderModule;
    vertexState.entryPoint = "vs_main";
    vertexState.bufferCount = 1;
    
    WGPUVertexBufferLayout vertexBufferLayout = WGPUVertex::GetLayout();
    vertexState.buffers = &vertexBufferLayout;
    
    WGPUFragmentState fragmentState = {};
    fragmentState.module = m_shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.targetCount = 1;
    
    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_BGRA8Unorm;
    colorTarget.blend = nullptr;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    fragmentState.targets = &colorTarget;
    
    WGPURenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.layout = pipelineLayout;
    pipelineDesc.vertex = vertexState;
    pipelineDesc.fragment = &fragmentState;
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDesc.primitive.cullMode = WGPUCullMode_None;
    
    m_trianglePipeline = wgpuDeviceCreateRenderPipeline(m_device->GetDevice(), &pipelineDesc);
    if (!m_trianglePipeline) {
        wgpuPipelineLayoutRelease(pipelineLayout);
        return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "Failed to create render pipeline");
    }
    
    // Create line pipeline
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_LineList;
    m_linePipeline = wgpuDeviceCreateRenderPipeline(m_device->GetDevice(), &pipelineDesc);
    if (!m_linePipeline) {
        wgpuPipelineLayoutRelease(pipelineLayout);
        return lab_result(LAB_ERROR_INITIALIZATION_FAILED, "Failed to create line pipeline");
    }
    
    wgpuPipelineLayoutRelease(pipelineLayout);
    return lab_result(LAB_ERROR_NONE);
}

lab_result WGPUBackend::Resize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    return lab_result(LAB_ERROR_NONE);
}

bool WGPUBackend::SupportsTextureFormat(TextureFormat format) const {
    switch (format) {
        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            return true;
        default:
            return false;
    }
}

bool WGPUBackend::SupportsBlendMode(BlendMode mode) const {
    return true; // All blend modes supported
}

} // namespace labfont
