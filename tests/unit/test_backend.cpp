#include <munit.h>
#include <labfont/labfont.h>
#include "../../src/core/backend.h"
#include "../../src/backends/cpu/cpu_backend.h"
#include "../utils/test_patterns.h"

using namespace labfont;

static MunitResult test_texture_creation(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<CPUBackend>();
    
    // Initialize backend
    labfont::lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Create texture
    TextureDesc desc = {
        .width = 256,
        .height = 256,
        .format = TextureFormat::RGBA8_UNORM,
        .renderTarget = false,
        .readback = true,
        .data = nullptr
    };
    
    std::shared_ptr<Texture> texture;
    result = backend->CreateTexture(desc, texture);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(texture.get());
    
    // Verify texture properties
    munit_assert_uint32(texture->GetWidth(), ==, 256);
    munit_assert_uint32(texture->GetHeight(), ==, 256);
    munit_assert_int(static_cast<int>(texture->GetFormat()), ==, static_cast<int>(TextureFormat::RGBA8_UNORM));
    munit_assert_false(texture->IsRenderTarget());
    munit_assert_true(texture->SupportsReadback());
    
    return MUNIT_OK;
}

static MunitResult test_texture_update(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<CPUBackend>();
    labfont::lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Create texture
    TextureDesc desc = {
        .width = 4,
        .height = 4,
        .format = TextureFormat::RGBA8_UNORM,
        .renderTarget = false,
        .readback = true,
        .data = nullptr
    };
    
    std::shared_ptr<Texture> texture;
    result = backend->CreateTexture(desc, texture);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
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
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Read back and verify
    std::vector<uint8_t> readback(pattern.size());
    result = backend->ReadbackTexture(texture.get(), readback.data(), readback.size());
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
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
    labfont::lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Create render target
    RenderTargetDesc desc = {
        .width = 512,
        .height = 512,
        .format = TextureFormat::RGBA8_UNORM,
        .hasDepth = true
    };
    
    std::shared_ptr<RenderTarget> target;
    result = backend->CreateRenderTarget(desc, target);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(target.get());
    
    // Verify render target properties
    munit_assert_uint32(target->GetWidth(), ==, 512);
    munit_assert_uint32(target->GetHeight(), ==, 512);
    munit_assert_int(static_cast<int>(target->GetFormat()), ==, static_cast<int>(TextureFormat::RGBA8_UNORM));
    munit_assert_true(target->HasDepth());
    
    // Verify textures
    munit_assert_not_null(target->GetColorTexture());
    munit_assert_not_null(target->GetDepthTexture());
    munit_assert_true(target->GetColorTexture()->IsRenderTarget());
    munit_assert_true(target->GetDepthTexture()->IsRenderTarget());
    
    // Set as current render target
    result = backend->SetRenderTarget(target.get());
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_ptr_equal(backend->GetCurrentRenderTarget(), target.get());
    
    return MUNIT_OK;
}

static MunitResult test_draw_commands(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<CPUBackend>();
    labfont::lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Create render target
    RenderTargetDesc rtDesc = {
        .width = 800,
        .height = 600,
        .format = TextureFormat::RGBA8_UNORM,
        .hasDepth = false
    };
    
    std::shared_ptr<RenderTarget> target;
    result = backend->CreateRenderTarget(rtDesc, target);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Set render target
    result = backend->SetRenderTarget(target.get());
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Begin frame
    result = backend->BeginFrame();
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Create some test vertices
    std::vector<Vertex> vertices = {
        {{0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
    };
    
    // Submit draw commands
    std::vector<DrawCommand> commands;
    
    // Clear command
    DrawCommand clear;
    clear.type = DrawCommandType::Clear;
    clear.clear.color[0] = 0.0f;
    clear.clear.color[1] = 0.0f;
    clear.clear.color[2] = 0.0f;
    clear.clear.color[3] = 1.0f;
    commands.push_back(clear);
    
    // Set blend mode
    DrawCommand blend;
    blend.type = DrawCommandType::SetBlendMode;
    blend.blend.mode = BlendMode::Alpha;
    commands.push_back(blend);
    
    // Draw triangles
    DrawCommand draw;
    draw.type = DrawCommandType::DrawTriangles;
    draw.triangles.vertexCount = 3;
    draw.triangles.vertexOffset = 0;
    draw.triangles.vertices = vertices.data();
    commands.push_back(draw);
    
    result = backend->SubmitCommands(commands);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // End frame
    result = backend->EndFrame();
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Verify submitted commands
    const auto& submitted = backend->GetSubmittedCommands();
    munit_assert_size(submitted.size(), ==, 3);
    munit_assert_int(static_cast<int>(submitted[0].type), ==, static_cast<int>(DrawCommandType::Clear));
    munit_assert_int(static_cast<int>(submitted[1].type), ==, static_cast<int>(DrawCommandType::SetBlendMode));
    munit_assert_int(static_cast<int>(submitted[2].type), ==, static_cast<int>(DrawCommandType::DrawTriangles));
    
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
