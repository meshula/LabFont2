#ifndef LABFONT_METAL_TYPES_H
#define LABFONT_METAL_TYPES_H

#ifdef __OBJC__

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace labfont {
namespace metal {

using MetalDeviceRef = id<MTLDevice>;
using MetalCommandQueueRef = id<MTLCommandQueue>;
using MetalLibraryRef = id<MTLLibrary>;
using MetalTextureRef = id<MTLTexture>;
using MetalRenderPipelineStateRef = id<MTLRenderPipelineState>;
using MetalDepthStencilStateRef = id<MTLDepthStencilState>;
using MetalRenderPassDescriptorRef = MTLRenderPassDescriptor*;
using MetalCommandBufferRef = id<MTLCommandBuffer>;
using MetalRenderCommandEncoderRef = id<MTLRenderCommandEncoder>;
using MetalBufferRef = id<MTLBuffer>;

} // namespace metal
} // namespace labfont

#else

namespace labfont {
namespace metal {

using MetalDeviceRef = void*;
using MetalCommandQueueRef = void*;
using MetalLibraryRef = void*;
using MetalTextureRef = void*;
using MetalRenderPipelineStateRef = void*;
using MetalDepthStencilStateRef = void*;
using MetalRenderPassDescriptorRef = void*;
using MetalCommandBufferRef = void*;
using MetalRenderCommandEncoderRef = void*;
using MetalBufferRef = void*;

} // namespace metal
} // namespace labfont

#endif

#endif // LABFONT_METAL_TYPES_H
