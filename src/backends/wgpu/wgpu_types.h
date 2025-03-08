#ifndef LABFONT_WGPU_TYPES_H
#define LABFONT_WGPU_TYPES_H

#include "core/internal_types.h"

// Forward declarations for WebGPU types to avoid include errors during development
#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
  #include <webgpu/webgpu.h>
  
  namespace labfont {
  
  inline WGPUTextureFormat GetWGPUTextureFormat(lab_texture_format format) {
      switch (format) {
          case LAB_TEXTURE_FORMAT_R8_UNORM:
              return WGPUTextureFormat_R8Unorm;
          case LAB_TEXTURE_FORMAT_RG8_UNORM:
              return WGPUTextureFormat_RG8Unorm;
          case LAB_TEXTURE_FORMAT_RGBA8_UNORM:
              return WGPUTextureFormat_RGBA8Unorm;
          case LAB_TEXTURE_FORMAT_R16F:
              return WGPUTextureFormat_R16Float;
          case LAB_TEXTURE_FORMAT_RG16F:
              return WGPUTextureFormat_RG16Float;
          case LAB_TEXTURE_FORMAT_RGBA16F:
              return WGPUTextureFormat_RGBA16Float;
          case LAB_TEXTURE_FORMAT_R32F:
              return WGPUTextureFormat_R32Float;
          case LAB_TEXTURE_FORMAT_RG32F:
              return WGPUTextureFormat_RG32Float;
          case LAB_TEXTURE_FORMAT_RGBA32F:
              return WGPUTextureFormat_RGBA32Float;
          case LAB_TEXTURE_FORMAT_D32F:
              return WGPUTextureFormat_Depth32Float;
          default:
              return WGPUTextureFormat_Undefined;
      }
  }
  
  inline WGPUTextureUsage GetWGPUTextureUsage(const TextureDesc& desc) {
      WGPUTextureUsageFlags usage = WGPUTextureUsage_TextureBinding;
      
      if (desc.renderTarget) {
          usage = usage | WGPUTextureUsage_RenderAttachment;
      }
      
      if (desc.readback) {
          usage = usage | WGPUTextureUsage_CopySrc;
      }
      
      if (desc.data) {
          usage = usage | WGPUTextureUsage_CopyDst;
      }
      
      return static_cast<WGPUTextureUsage>(usage);
  }
  
  inline WGPUTextureDescriptor GetWGPUTextureDescriptor(const TextureDesc& desc) {
      WGPUTextureDescriptor wgpuDesc = {};
      wgpuDesc.usage = GetWGPUTextureUsage(desc);
      wgpuDesc.dimension = WGPUTextureDimension_2D;
      wgpuDesc.size = {
          .width = desc.width,
          .height = desc.height,
          .depthOrArrayLayers = 1
      };
      wgpuDesc.format = GetWGPUTextureFormat(desc.format);
      wgpuDesc.mipLevelCount = 1;
      wgpuDesc.sampleCount = 1;
      return wgpuDesc;
  }
  
  } // namespace labfont
#else
  // Stub implementations for development without Emscripten
  namespace labfont {
  
  // Forward declarations of WebGPU enums and types
  enum WGPUTextureFormat {
      WGPUTextureFormat_Undefined,
      WGPUTextureFormat_R8Unorm,
      WGPUTextureFormat_RG8Unorm,
      WGPUTextureFormat_RGBA8Unorm,
      WGPUTextureFormat_R16Float,
      WGPUTextureFormat_RG16Float,
      WGPUTextureFormat_RGBA16Float,
      WGPUTextureFormat_R32Float,
      WGPUTextureFormat_RG32Float,
      WGPUTextureFormat_RGBA32Float,
      WGPUTextureFormat_Depth32Float
  };
  
  enum WGPUTextureUsage {
      WGPUTextureUsage_TextureBinding = 0x01,
      WGPUTextureUsage_RenderAttachment = 0x02,
      WGPUTextureUsage_CopySrc = 0x04,
      WGPUTextureUsage_CopyDst = 0x08
  };
  
  typedef uint32_t WGPUTextureUsageFlags;
  
  enum WGPUTextureDimension {
      WGPUTextureDimension_1D,
      WGPUTextureDimension_2D,
      WGPUTextureDimension_3D
  };
  
  struct WGPUExtent3D {
      uint32_t width;
      uint32_t height;
      uint32_t depthOrArrayLayers;
  };
  
  struct WGPUTextureDescriptor {
      WGPUTextureUsage usage;
      WGPUTextureDimension dimension;
      WGPUExtent3D size;
      WGPUTextureFormat format;
      uint32_t mipLevelCount;
      uint32_t sampleCount;
  };
  
  // Stub implementations of conversion functions
  inline WGPUTextureFormat GetWGPUTextureFormat(lab_texture_format format) {
      // Stub implementation for development
      return WGPUTextureFormat_Undefined;
  }
  
  inline WGPUTextureUsage GetWGPUTextureUsage(const TextureDesc& desc) {
      // Stub implementation for development
      return static_cast<WGPUTextureUsage>(0);
  }
  
  inline WGPUTextureDescriptor GetWGPUTextureDescriptor(const TextureDesc& desc) {
      // Stub implementation for development
      WGPUTextureDescriptor wgpuDesc = {};
      return wgpuDesc;
  }
  
  } // namespace labfont
#endif

#endif // LABFONT_WGPU_TYPES_H
