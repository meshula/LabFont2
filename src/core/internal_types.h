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

// Ensure this matches lab_draw_command_type in labfont_types.h
enum class DrawCommandType {
    Clear = LAB_DRAW_COMMAND_CLEAR,
    DrawTriangles = LAB_DRAW_COMMAND_TRIANGLES,
    DrawLines = LAB_DRAW_COMMAND_LINES,
    SetBlendMode,  // Extended commands for internal use
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
    DrawCommandType type;  // Using our C++ enum instead of the C enum
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
        struct {
            BlendMode mode;
        } blend;
        struct {
            int32_t x;
            int32_t y;
            uint32_t width;
            uint32_t height;
        } scissor;
        struct {
            float x;
            float y;
            float width;
            float height;
        } viewport;
    };

    // Default constructor
    DrawCommand() : type(DrawCommandType::Clear) {
        clear.color[0] = 0.0f;
        clear.color[1] = 0.0f;
        clear.color[2] = 0.0f;
        clear.color[3] = 1.0f;
    }

    // Constructor to convert from C API type
    DrawCommand(const lab_draw_command& cmd) {
        // Convert C enum to C++ enum
        switch (cmd.type) {
            case LAB_DRAW_COMMAND_CLEAR:
                type = DrawCommandType::Clear;
                std::memcpy(&clear, &cmd.clear, sizeof(clear));
                break;
            case LAB_DRAW_COMMAND_TRIANGLES:
                type = DrawCommandType::DrawTriangles;
                std::memcpy(&triangles, &cmd.triangles, sizeof(triangles));
                break;
            case LAB_DRAW_COMMAND_LINES:
                type = DrawCommandType::DrawLines;
                std::memcpy(&lines, &cmd.lines, sizeof(lines));
                break;
        }
    }

    // Static factory methods for internal commands
    static DrawCommand CreateBlendCommand(BlendMode mode) {
        DrawCommand cmd;
        cmd.type = DrawCommandType::SetBlendMode;
        cmd.blend.mode = mode;
        return cmd;
    }

    static DrawCommand CreateScissorCommand(int32_t x, int32_t y, uint32_t width, uint32_t height) {
        DrawCommand cmd;
        cmd.type = DrawCommandType::SetScissor;
        cmd.scissor.x = x;
        cmd.scissor.y = y;
        cmd.scissor.width = width;
        cmd.scissor.height = height;
        return cmd;
    }

    static DrawCommand CreateViewportCommand(float x, float y, float width, float height) {
        DrawCommand cmd;
        cmd.type = DrawCommandType::SetViewport;
        cmd.viewport.x = x;
        cmd.viewport.y = y;
        cmd.viewport.width = width;
        cmd.viewport.height = height;
        return cmd;
    }
};

} // namespace labfont

#endif // LABFONT_INTERNAL_TYPES_H
