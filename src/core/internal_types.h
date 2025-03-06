#ifndef LABFONT_INTERNAL_TYPES_H
#define LABFONT_INTERNAL_TYPES_H

#include "labfont/labfont_types.h"
#include <cstring>

namespace labfont {

enum class TextureFormat {
    Unknown,
    R8_UNORM,
    RG8_UNORM,
    RGBA8_UNORM,
    R16F,
    RG16F,
    RGBA16F,
    R32F,
    RG32F,
    RGBA32F,
    D32F     // 32-bit floating point depth format with no stencil
};

struct TextureDesc {
    uint32_t width;
    uint32_t height;
    TextureFormat format;
    const void* data;
    bool renderTarget;
    bool readback;
    size_t dataSize;
};

enum class DrawCommandType {
    Clear,
    DrawTriangles,
    DrawLines,
    SetBlendMode,
    SetScissor,
    SetViewport
};

enum class BlendMode {
    None,
    Alpha,
    Additive,
    Multiply,
    Screen
};


struct RenderTargetDesc {
    uint32_t width;
    uint32_t height;
    TextureFormat format;
    bool hasDepth;
};

struct DrawCommand {
    lab_draw_command_type type;
    union {
        struct {
            float color[4];
        } clear;
        struct {
            const lab_vertex_2TC* vertices;
            uint32_t vertexCount;
        } triangles;
        struct {
            const lab_vertex_2TC* vertices;
            uint32_t vertexCount;
            float lineWidth;
        } lines;
    };

    // Constructor to convert from C API type
    DrawCommand(const lab_draw_command& cmd) : type(cmd.type) {
        switch (type) {
            case LAB_DRAW_COMMAND_CLEAR:
                std::memcpy(&clear, &cmd.clear, sizeof(clear));
                break;
            case LAB_DRAW_COMMAND_TRIANGLES:
                std::memcpy(&triangles, &cmd.triangles, sizeof(triangles));
                break;
            case LAB_DRAW_COMMAND_LINES:
                std::memcpy(&lines, &cmd.lines, sizeof(lines));
                break;
        }
    }
};

} // namespace labfont

#endif // LABFONT_INTERNAL_TYPES_H
