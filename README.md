# LabFont v2

A modern text rendering and immediate mode drawing library with multiple backend support.

## Features

- Pure C interface with C++ implementation
- Multiple backend support (Metal, WGPU, Vulkan, DX11)
- Text rendering with styling and layout
- Immediate mode drawing operations
- Thread-safe design

## Dependencies

The following dependencies are included in the `third_party` directory:
- [fontstash](https://github.com/memononen/fontstash) - Text rendering and font management
- [stb_truetype](https://github.com/nothings/stb) - TrueType font loading
- [stb_image](https://github.com/nothings/stb) - Image loading for textures

Test dependencies:
- [munit](https://github.com/nemequ/munit) - Unit testing framework (fetched automatically by CMake)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

### Build Options

- `LABFONT_BUILD_TESTS` - Build tests (ON by default)
- `LABFONT_BUILD_EXAMPLES` - Build examples (ON by default)
- `LABFONT_ENABLE_METAL` - Enable Metal backend
- `LABFONT_ENABLE_WGPU` - Enable WebGPU backend
- `LABFONT_ENABLE_VULKAN` - Enable Vulkan backend
- `LABFONT_ENABLE_DX11` - Enable DirectX 11 backend

Example:
```bash
cmake .. -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=ON
```

## Testing

```bash
cd build
ctest
```

## License

TBD
