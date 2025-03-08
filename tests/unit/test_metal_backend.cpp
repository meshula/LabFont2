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
    
    // In a test environment, we may not have the Metal shader library available,
    // so we accept either success or initialization failed
    if (result != LAB_RESULT_OK) {
        munit_assert_int(result, ==, LAB_RESULT_INITIALIZATION_FAILED);
        munit_assert_not_null(result.message.c_str());
        
        // Display the error message with clear formatting
        printf("Metal initialization failed as expected: %s\n", result.message.c_str());
        
        // The detailed error information is captured in the stderr output
        // which is redirected to the result.message in MetalBackend::Initialize
        
        return MUNIT_SKIP;  // Skip the test since we can't proceed without initialization
    }
    
    // If we get here, Metal initialized successfully
    printf("Metal initialized successfully\n");
    return MUNIT_OK;
}

static MunitResult test_metal_render_target(const MunitParameter params[], void* data) {
    (void)params;
    (void)data;
    
    auto backend = std::make_unique<metal::MetalBackend>();
    labfont::lab_result result = backend->Initialize(800, 600);
    
    // Skip test if initialization fails
    if (result != LAB_RESULT_OK) {
        printf("Skipping render target test due to initialization failure: %s\n", result.message.c_str());
        return MUNIT_SKIP;
    }
    
    printf("Metal render target test running with initialized backend\n");
    
    // Create render target
    RenderTargetDesc desc = {
        .width = 512,
        .height = 512,
        .format = TextureFormat::RGBA8_UNORM,
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

static MunitResult test_metal_texture(const MunitParameter params[], void* data) {
    (void)params;
    (void)data;
    
    auto backend = std::make_unique<metal::MetalBackend>();
    labfont::lab_result result = backend->Initialize(800, 600);
    
    // Skip test if initialization fails
    if (result != LAB_RESULT_OK) {
        printf("Skipping texture test due to initialization failure: %s\n", result.message.c_str());
        return MUNIT_SKIP;
    }
    
    printf("Metal texture test running with initialized backend\n");
    
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
    munit_assert_int(result, ==, LAB_RESULT_OK);
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
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Read back texture
    std::vector<uint8_t> readback(pattern.size());
    result = backend->ReadbackTexture(texture.get(), readback.data(), readback.size());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
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
