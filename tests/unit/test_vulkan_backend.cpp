#include <munit.h>
#include <labfont/labfont.h>
#include "../../src/backends/vulkan/vulkan_backend.h"
#include "../../src/backends/vulkan/vulkan_device.h"
#include "../utils/test_patterns.h"

using namespace labfont;

static void* test_setup(const MunitParameter params[], void* user_data) {
    (void)params;
    (void)user_data;
    return nullptr;
}

static void test_tear_down(void* fixture) {
    (void)fixture;
}

static MunitResult test_vulkan_initialization(const MunitParameter params[], void* data) {
    (void)params;
    (void)data;
    
    auto backend = std::make_unique<vulkan::VulkanBackend>();
    
    // Initialize backend
    lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    return MUNIT_OK;
}

static MunitResult test_vulkan_render_target(const MunitParameter params[], void* data) {
    (void)params;
    (void)data;
    
    auto backend = std::make_unique<vulkan::VulkanBackend>();
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
    
    // Set as current render target
    result = backend->SetRenderTarget(target.get());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Begin frame
    result = backend->BeginFrame();
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Create test vertices for a triangle
    lab_vertex_2TC vertices[3] = {
        {{-0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{ 0.0f,  0.5f}, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
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
    
    return MUNIT_OK;
}

static MunitResult test_vulkan_texture(const MunitParameter params[], void* data) {
    (void)params;
    (void)data;
    
    auto backend = std::make_unique<vulkan::VulkanBackend>();
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
    
    // Generate test pattern
    uint8_t color1[] = {255, 0, 0, 255};  // Red
    uint8_t color2[] = {0, 255, 0, 255};  // Green
    std::vector<uint8_t> pattern = PatternGenerator::GenerateCheckerboard<uint8_t>(
        256, 256, 32,
        color1,
        color2,
        4  // RGBA
    );
    
    // Update texture
    result = backend->UpdateTexture(texture.get(), pattern.data(), pattern.size());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Read back texture
    std::vector<uint8_t> readback(pattern.size());
    result = backend->ReadbackTexture(texture.get(), readback.data(), readback.size());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Compare patterns
    bool matches = PixelComparator::CompareBuffers<uint8_t>(
        pattern.data(),
        readback.data(),
        256 * 256,  // Pixel count
        4,          // RGBA channels
        2           // Tolerance
    );
    munit_assert_true(matches);
    
    return MUNIT_OK;
}

static MunitTest vulkan_tests[] = {
    {
        (char*)"/initialization",
        test_vulkan_initialization,
        test_setup,
        test_tear_down,
        MUNIT_TEST_OPTION_NONE,
        nullptr
    },
    {
        (char*)"/render_target",
        test_vulkan_render_target,
        test_setup,
        test_tear_down,
        MUNIT_TEST_OPTION_NONE,
        nullptr
    },
    {
        (char*)"/texture",
        test_vulkan_texture,
        test_setup,
        test_tear_down,
        MUNIT_TEST_OPTION_NONE,
        nullptr
    },
    { nullptr, nullptr, nullptr, nullptr, MUNIT_TEST_OPTION_NONE, nullptr }
};

extern "C" {
    MunitSuite vulkan_suite = {
        (char*)"/vulkan",
        vulkan_tests,
        nullptr,
        1,
        MUNIT_SUITE_OPTION_NONE
    };
}
