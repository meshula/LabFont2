#ifndef LABFONT_TYPES_H
#define LABFONT_TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
typedef enum lab_result
{
    LAB_RESULT_OK = 0,
    LAB_RESULT_INVALID_PARAMETER = -1,
    LAB_RESULT_OUT_OF_MEMORY = -2,
    LAB_RESULT_BACKEND_ERROR = -3,
    LAB_RESULT_FONT_LOAD_FAILED = -4,
    LAB_RESULT_NOT_INITIALIZED = -5,
    LAB_RESULT_UNSUPPORTED_BACKEND = -6,
    LAB_RESULT_INITIALIZATION_FAILED = -7,
    LAB_RESULT_INVALID_OPERATION = -8,
    LAB_RESULT_COMMAND_BUFFER = -9,
    LAB_RESULT_UNSUPPORTED_FORMAT = -10,
    LAB_RESULT_DEVICE_LOST = -11,
    LAB_RESULT_STATE_NO_RENDER_TARGET_SET = -12,
    LAB_RESULT_INVALID_DIMENSION = -13,
    LAB_RESULT_INVALID_TEXTURE = -14,
    LAB_RESULT_READBACK_NOT_SUPPORTED = -15,
    LAB_RESULT_INVALID_RENDER_TARGET = -16,
    LAB_RESULT_INVALID_CONTEXT = -17,
    LAB_RESULT_INVALID_RESOURCE_NAME = -18,
    LAB_RESULT_DUPLICATE_RESOURCE_NAME = -19,
    LAB_RESULT_INVALID_BUFFER_SIZE = -20,
    LAB_RESULT_INVALID_BUFFER = -21,
    LAB_RESULT_TEXTURE_CREATION_FAILED = -22,
    LAB_RESULT_SHADER_LIBRARY_INITIALIZATION_FAILED = -23,
    LAB_RESULT_DEVICE_INITIALIZATION_FAILED = -24,
    LAB_RESULT_RENDERTARGET_INITIALIZATION_FAILED = -25,
    LAB_RESULT_COMMAND_BUFFER_INITIALIZATION_FAILED = -26,
    LAB_RESULT_COMMAND_ENCODER_INITIALIZATION_FAILED = -27,
    LAB_RESULT_INVALID_COMMAND_BUFFER = -28,
} lab_result;

/* Texture formats */
typedef enum lab_texture_format {
    LAB_TEXTURE_FORMAT_UNKNOWN,
    LAB_TEXTURE_FORMAT_R8_UNORM,
    LAB_TEXTURE_FORMAT_RG8_UNORM,
    LAB_TEXTURE_FORMAT_RGBA8_UNORM,
    LAB_TEXTURE_FORMAT_BGRA8_UNORM_SRGB,  /* BGRA format with sRGB encoding */
    LAB_TEXTURE_FORMAT_R16F,
    LAB_TEXTURE_FORMAT_RG16F,
    LAB_TEXTURE_FORMAT_RGBA16F,
    LAB_TEXTURE_FORMAT_R32F,
    LAB_TEXTURE_FORMAT_RG32F,
    LAB_TEXTURE_FORMAT_RGBA32F,
    LAB_TEXTURE_FORMAT_D32F     /* 32-bit floating point depth format with no stencil */
} lab_texture_format;

/* Backend types */
typedef enum lab_backend_type
{
    LAB_BACKEND_METAL,
    LAB_BACKEND_WGPU,
    LAB_BACKEND_VULKAN,
    LAB_BACKEND_DX11,
    LAB_BACKEND_CPU
} lab_backend_type;

/* Color type */
typedef struct lab_color
{
    float r, g, b, a;
} lab_color;

/* Backend description */
typedef struct lab_backend_desc {
    lab_backend_type type;     /* Backend type (Metal, WebGPU, etc.) */
    unsigned int width;        /* Initial viewport width */
    unsigned int height;       /* Initial viewport height */
    void* native_window;      /* Native window handle (platform-specific) */
} lab_backend_desc;

/* Context description */
typedef struct lab_context_desc {
    unsigned int width;         /* Initial viewport width */
    unsigned int height;        /* Initial viewport height */
    void* native_window;       /* Native window handle (platform-specific) */
    unsigned int max_vertices;  /* Maximum number of vertices for immediate mode drawing */
    unsigned int atlas_width;   /* Width of the font atlas texture */
    unsigned int atlas_height;  /* Height of the font atlas texture */
} lab_context_desc;

/* Vertex type with position, texture coordinates, and color */
typedef struct lab_vertex_2TC {
    float position[2];  /* x, y */
    float texcoord[2];  /* u, v */
    float color[4];     /* r, g, b, a */
} lab_vertex_2TC;

/* Opaque handle types */
typedef struct lab_context_t* lab_context;
typedef struct lab_font_t* lab_font;
typedef struct lab_texture_t* lab_texture;
typedef struct lab_buffer_t* lab_buffer;
typedef struct lab_render_target_t* lab_render_target;

/* Draw command types */
typedef enum lab_draw_command_type {
    LAB_DRAW_COMMAND_CLEAR,
    LAB_DRAW_COMMAND_TRIANGLES,
    LAB_DRAW_COMMAND_LINES,
    LAB_DRAW_COMMAND_BIND_TEXTURE,
} lab_draw_command_type;

/* Draw command data */
typedef struct lab_draw_command {
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
        struct {
            const lab_texture texture;
        } bind_texture;
    };
} lab_draw_command;

/* Render target description */
typedef struct lab_render_target_desc {
    uint32_t width;
    uint32_t height;
    lab_texture_format format;
    bool hasDepth;
} lab_render_target_desc;


/* Resource descriptors */
typedef struct lab_texture_desc {
    unsigned int width;
    unsigned int height;
    lab_texture_format format;
    const void* initial_data;
} lab_texture_desc;

typedef struct lab_buffer_desc {
    size_t size;
    bool dynamic;
    const void* initial_data;
} lab_buffer_desc;

/* Text alignment */
typedef enum lab_text_align
{
    LAB_TEXT_ALIGN_LEFT,
    LAB_TEXT_ALIGN_CENTER,
    LAB_TEXT_ALIGN_RIGHT
} lab_text_align;

/* Text style flags */
typedef enum lab_text_style
{
    LAB_TEXT_STYLE_NORMAL = 0,
    LAB_TEXT_STYLE_BOLD = 1 << 0,
    LAB_TEXT_STYLE_ITALIC = 1 << 1,
    LAB_TEXT_STYLE_UNDERLINE = 1 << 2
} lab_text_style;

/* Memory categories */
typedef enum lab_memory_category
{
    LAB_MEMORY_GENERAL,
    LAB_MEMORY_GRAPHICS,
    LAB_MEMORY_TEXT,
    LAB_MEMORY_RESOURCES,
    LAB_MEMORY_TEMPORARY
} lab_memory_category;

/* Memory statistics */
typedef struct lab_memory_stats
{
    size_t totalAllocated;
    size_t totalFreed;
    size_t currentUsage;
    size_t peakUsage;
    size_t categoryUsage[5];  /* One for each lab_memory_category */
} lab_memory_stats;

#ifdef __cplusplus
}
#endif

#endif /* LABFONT_TYPES_H */
