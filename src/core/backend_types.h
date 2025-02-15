#ifndef LABFONT_BACKEND_TYPES_H
#define LABFONT_BACKEND_TYPES_H

#include <cstdint>

namespace labfont {

// Texture formats
enum class TextureFormat {
    R8_UNORM,      // 8-bit single channel
    RG8_UNORM,     // 8-bit dual channel
    RGBA8_UNORM,   // 8-bit RGBA
    R16F,          // 16-bit float single channel
    RG16F,         // 16-bit float dual channel
    RGBA16F,       // 16-bit float RGBA
    R32F,          // 32-bit float single channel
    RG32F,         // 32-bit float dual channel
    RGBA32F        // 32-bit float RGBA
};

// Blend modes
enum class BlendMode {
    None,          // No blending
    Alpha,         // Standard alpha blending
    Additive,      // Additive blending
    Multiply,      // Multiplicative blending
    Screen         // Screen blending
};

// Texture description
struct TextureDesc {
    uint32_t width;
    uint32_t height;
    TextureFormat format;
    bool renderTarget;     // Can this texture be used as a render target?
    bool readback;        // Can this texture be read back to CPU?
    const void* data;     // Initial data (optional)
};

// Render target description
struct RenderTargetDesc {
    uint32_t width;
    uint32_t height;
    TextureFormat format;
    bool hasDepth;        // Whether to create a depth buffer
};

// Vertex format
struct Vertex {
    float position[2];    // x, y position
    float texcoord[2];    // u, v texture coordinates
    float color[4];       // r, g, b, a color
};

// Draw command types
enum class DrawCommandType {
    Clear,          // Clear render target
    DrawTriangles,  // Draw triangles from vertex buffer
    DrawLines,      // Draw lines from vertex buffer
    SetBlendMode,   // Set blend mode
    SetScissor,     // Set scissor rectangle
    SetViewport     // Set viewport
};

// Draw command parameters
struct ClearParams {
    float color[4];
};

struct DrawTrianglesParams {
    uint32_t vertexCount;
    uint32_t vertexOffset;
    const Vertex* vertices;
};

struct DrawLinesParams {
    uint32_t vertexCount;
    uint32_t vertexOffset;
    const Vertex* vertices;
    float lineWidth;
};

struct BlendModeParams {
    BlendMode mode;
};

struct ScissorParams {
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
};

struct ViewportParams {
    float x;
    float y;
    float width;
    float height;
};

// Draw command union
struct DrawCommand {
    DrawCommandType type;
    union {
        ClearParams clear;
        DrawTrianglesParams triangles;
        DrawLinesParams lines;
        BlendModeParams blend;
        ScissorParams scissor;
        ViewportParams viewport;
    };
};

} // namespace labfont

#endif // LABFONT_BACKEND_TYPES_H
