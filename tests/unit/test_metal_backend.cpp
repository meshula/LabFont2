#include <munit.h>
#include <labfont/labfont.h>
#include "../../src/backends/metal/metal_backend.h"

using namespace labfont;

static void* test_setup(const MunitParameter params[], void* user_data) {
    (void)params;
    (void)user_data;
    return nullptr;
}

static void test_tear_down(void* fixture) {
    (void)fixture;
}

static MunitResult test_metal_initialization(const MunitParameter params[], void* data) {
    (void)params;
    (void)data;
    
    auto backend = std::make_unique<metal::MetalBackend>();
    
    // Initialize backend
    labfont::lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    return MUNIT_OK;
}

static MunitResult test_metal_render_target(const MunitParameter params[], void* data) {
    (void)params;
    (void)data;
    
    auto backend = std::make_unique<metal::MetalBackend>();
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
    
    // Set as current render target
    result = backend->SetRenderTarget(target.get());
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Begin frame
    result = backend->BeginFrame();
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Create test vertices for a triangle
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{ 0.0f,  0.5f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
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
    
    // Draw triangle
    DrawCommand draw;
    draw.type = DrawCommandType::DrawTriangles;
    draw.triangles.vertices = vertices.data();
    draw.triangles.vertexCount = vertices.size();
    commands.push_back(draw);
    
    result = backend->SubmitCommands(commands);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // End frame
    result = backend->EndFrame();
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    return MUNIT_OK;
}

static MunitResult test_metal_texture(const MunitParameter params[], void* data) {
    (void)params;
    (void)data;
    
    auto backend = std::make_unique<metal::MetalBackend>();
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
    
    // Generate test pattern
    std::vector<uint8_t> pattern(256 * 256 * 4, 0);
    for (size_t i = 0; i < pattern.size(); i += 4) {
        pattern[i] = 255;     // R
        pattern[i + 1] = 0;   // G
        pattern[i + 2] = 0;   // B
        pattern[i + 3] = 255; // A
    }
    
    // Update texture
    result = backend->UpdateTexture(texture.get(), pattern.data(), pattern.size());
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Read back texture
    std::vector<uint8_t> readback(pattern.size());
    result = backend->ReadbackTexture(texture.get(), readback.data(), readback.size());
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Compare patterns
    for (size_t i = 0; i < pattern.size(); i++) {
        munit_assert_uint8(readback[i], ==, pattern[i]);
    }
    
    return MUNIT_OK;
}

static MunitTest metal_tests[] = {
    {
        (char*)"/initialization",
        test_metal_initialization,
        test_setup,
        test_tear_down,
        MUNIT_TEST_OPTION_NONE,
        nullptr
    },
    {
        (char*)"/render_target",
        test_metal_render_target,
        test_setup,
        test_tear_down,
        MUNIT_TEST_OPTION_NONE,
        nullptr
    },
    {
        (char*)"/texture",
        test_metal_texture,
        test_setup,
        test_tear_down,
        MUNIT_TEST_OPTION_NONE,
        nullptr
    },
    { nullptr, nullptr, nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr }
};

extern "C" {
    MunitSuite metal_suite = {
        (char*)"/metal",
        metal_tests,
        nullptr,
        1,
        MUNIT_SUITE_OPTION_NONE
    };
}
