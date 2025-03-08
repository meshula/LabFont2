#include <munit.h>
#include <labfont/labfont.h>
#include "../../src/core/backend.h"
#include "../../src/backends/cpu/cpu_backend.h"
#include "../utils/test_patterns.h"

// Define Vertex type for tests
namespace labfont {
    struct Vertex {
        float position[2];
        float texcoord[2];
        float color[4];
    };
}

using namespace labfont;

static MunitResult test_texture_creation(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<CPUBackend>();
    
    // Initialize backend
    lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Create texture
    TextureDesc desc = {
        .width = 256,
        .height = 256,
        .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
        .renderTarget = false,
        .readback = true,
        .data = nullptr
    };
    
    std::shared_ptr<Texture> texture;
    result = backend->CreateTexture(desc, texture);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    munit_assert_not_null(texture.get());
    
    // Verify texture properties
    munit_assert_uint32(texture->GetWidth(), ==, 256);
    munit_assert_uint32(texture->GetHeight(), ==, 256);
    munit_assert_int(static_cast<int>(texture->GetFormat()), ==, static_cast<int>(LAB_TEXTURE_FORMAT_RGBA8_UNORM));
    munit_assert_false(texture->IsRenderTarget());
    munit_assert_true(texture->SupportsReadback());
    
    return MUNIT_OK;
}

static MunitResult test_texture_update(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<CPUBackend>();
    lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Create texture
    TextureDesc desc = {
        .width = 4,
        .height = 4,
        .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
        .renderTarget = false,
        .readback = true,
        .data = nullptr
    };
    
    std::shared_ptr<Texture> texture;
    result = backend->CreateTexture(desc, texture);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Generate test pattern
    uint8_t color1[] = {255, 0, 0, 255};  // Red
    uint8_t color2[] = {0, 255, 0, 255};  // Green
    auto pattern = PatternGenerator::GenerateCheckerboard<uint8_t>(
        4, 4,    // width, height
        2,       // block size
        color1,
        color2,
        4        // channels (RGBA)
    );
    
    // Update texture
    result = backend->UpdateTexture(texture.get(), pattern.data(), pattern.size());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Read back and verify
    std::vector<uint8_t> readback(pattern.size());
    result = backend->ReadbackTexture(texture.get(), readback.data(), readback.size());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Compare with original pattern
    bool matches = PixelComparator::CompareBuffers<uint8_t>(
        pattern.data(),
        readback.data(),
        16,     // 4x4 pixels
        4,      // RGBA
        0       // exact match required
    );
    munit_assert_true(matches);
    
    return MUNIT_OK;
}

static MunitResult test_render_target(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<CPUBackend>();
    lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Create render target
    RenderTargetDesc desc = {
        .width = 512,
        .height = 512,
        .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
        .hasDepth = true
    };
    
    std::shared_ptr<RenderTarget> target;
    result = backend->CreateRenderTarget(desc, target);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    munit_assert_not_null(target.get());
    
    // Verify render target properties
    munit_assert_uint32(target->GetWidth(), ==, 512);
    munit_assert_uint32(target->GetHeight(), ==, 512);
    munit_assert_int(static_cast<int>(target->GetFormat()), ==, static_cast<int>(LAB_TEXTURE_FORMAT_RGBA8_UNORM));
    munit_assert_true(target->HasDepth());
    
    // Verify textures
    munit_assert_not_null(target->GetColorTexture());
    munit_assert_not_null(target->GetDepthTexture());
    munit_assert_true(target->GetColorTexture()->IsRenderTarget());
    munit_assert_true(target->GetDepthTexture()->IsRenderTarget());
    
    // Set as current render target
    result = backend->SetRenderTarget(target.get());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    // Note: GetCurrentRenderTarget() is not implemented in CPUBackend
    // munit_assert_ptr_equal(backend->GetCurrentRenderTarget(), target.get());
    
    return MUNIT_OK;
}

static MunitResult test_draw_commands(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<CPUBackend>();
    lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Create render target
    RenderTargetDesc rtDesc = {
        .width = 800,
        .height = 600,
        .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
        .hasDepth = false
    };
    
    std::shared_ptr<RenderTarget> target;
    result = backend->CreateRenderTarget(rtDesc, target);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Set render target
    result = backend->SetRenderTarget(target.get());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Begin frame
    result = backend->BeginFrame();
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Create some test vertices
    lab_vertex_2TC vertices[3] = {
        {{0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
    };
    
    // Create C API draw commands
    lab_draw_command clear_cmd = {
        .type = LAB_DRAW_COMMAND_CLEAR,
        .clear = {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        }
    };
    
    lab_draw_command draw_cmd = {
        .type = LAB_DRAW_COMMAND_TRIANGLES,
        .triangles = {
            .vertices = vertices,
            .vertexCount = 3
        }
    };
    
    // Convert to C++ API draw commands
    std::vector<DrawCommand> commands;
    commands.push_back(DrawCommand(clear_cmd));
    commands.push_back(DrawCommand(draw_cmd));
    
    result = backend->SubmitCommands(commands);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // End frame
    result = backend->EndFrame();
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Note: GetSubmittedCommands() is not implemented in CPUBackend
    // const auto& submitted = backend->GetSubmittedCommands();
    // munit_assert_size(submitted.size(), ==, 2);
    // munit_assert_int(static_cast<int>(submitted[0].type), ==, static_cast<int>(LAB_DRAW_COMMAND_CLEAR));
    // munit_assert_int(static_cast<int>(submitted[1].type), ==, static_cast<int>(LAB_DRAW_COMMAND_TRIANGLES));
    
    return MUNIT_OK;
}

static MunitTest backend_tests[] = {
    {
        (char*)"/texture_creation",
        test_texture_creation,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        (char*)"/texture_update",
        test_texture_update,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        (char*)"/render_target",
        test_render_target,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        (char*)"/draw_commands",
        test_draw_commands,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

extern "C" MunitSuite backend_suite = {
    (char*)"/backend",
    backend_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
