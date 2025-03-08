#include <munit.h>
#include <labfont/labfont.h>
#include "../../src/backends/wgpu/wgpu_backend.h"
#include "../utils/test_patterns.h"

using namespace labfont;

static MunitResult test_wgpu_initialization(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<WGPUBackend>();
    
    // Initialize backend
    lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Verify supported features
    munit_assert_true(backend->SupportsTextureFormat(LAB_TEXTURE_FORMAT_RGBA8_UNORM));
    munit_assert_true(backend->SupportsBlendMode(BlendMode::Alpha));
    munit_assert_uint32(backend->GetMaxTextureSize(), >=, 8192);
    
    return MUNIT_OK;
}

static MunitResult test_wgpu_texture_operations(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<WGPUBackend>();
    backend->Initialize(800, 600);
    
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
    lab_result result = backend->CreateTexture(desc, texture);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    munit_assert_not_null(texture.get());
    
    // Generate test pattern
    uint8_t color1[] = {255, 0, 0, 255};  // Red
    uint8_t color2[] = {0, 255, 0, 255};  // Green
    auto pattern = PatternGenerator::GenerateCheckerboard<uint8_t>(
        256, 256,  // width, height
        32,        // block size
        color1,
        color2,
        4         // channels (RGBA)
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
        256 * 256,  // total pixels
        4,          // RGBA
        1           // allow 1 unit difference for potential precision loss
    );
    munit_assert_true(matches);
    
    return MUNIT_OK;
}

static MunitResult test_wgpu_render_target(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<WGPUBackend>();
    backend->Initialize(800, 600);
    
    // Create render target
    RenderTargetDesc desc = {
        .width = 512,
        .height = 512,
        .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
        .hasDepth = true
    };
    
    std::shared_ptr<RenderTarget> target;
    lab_result result = backend->CreateRenderTarget(desc, target);
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
    
    // Read back render target
    std::vector<uint8_t> readback(512 * 512 * 4);
    result = backend->ReadbackTexture(target->GetColorTexture(), readback.data(), readback.size());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Verify some pixels are colored (not just black)
    bool hasColoredPixels = false;
    for (size_t i = 0; i < readback.size(); i += 4) {
        if (readback[i] > 0 || readback[i + 1] > 0 || readback[i + 2] > 0) {
            hasColoredPixels = true;
            break;
        }
    }
    munit_assert_true(hasColoredPixels);
    
    return MUNIT_OK;
}

static MunitResult test_wgpu_blend_modes(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<WGPUBackend>();
    backend->Initialize(800, 600);
    
    // Create render target
    RenderTargetDesc desc = {
        .width = 256,
        .height = 256,
        .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
        .hasDepth = false
    };
    
    std::shared_ptr<RenderTarget> target;
    lab_result result = backend->CreateRenderTarget(desc, target);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    backend->SetRenderTarget(target.get());
    backend->BeginFrame();
    
    // Test vertices for overlapping squares
    lab_vertex_2TC redSquare[4] = {
        {{-0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.5f}},
        {{ 0.0f, -0.5f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.5f}},
        {{-0.5f,  0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 0.5f}},
        {{ 0.0f,  0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 0.5f}}
    };
    
    lab_vertex_2TC blueSquare[4] = {
        {{-0.25f, -0.25f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.5f}},
        {{ 0.25f, -0.25f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.5f}},
        {{-0.25f,  0.25f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.5f}},
        {{ 0.25f,  0.25f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.5f}}
    };
    
    // Create C API draw commands
    lab_draw_command clear_cmd = {
        .type = LAB_DRAW_COMMAND_CLEAR,
        .clear = {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        }
    };
    
    // Note: We can't directly create a SetBlendMode command in C API
    // We'll use the C++ API's DrawCommand constructor for the triangles commands
    
    // Convert to C++ API draw commands
    std::vector<DrawCommand> commands;
    commands.push_back(DrawCommand(clear_cmd));
    
    // For blend mode and other commands that don't have C API equivalents,
    // we'll need to create them differently or modify the test
    
    // Create triangle commands for red and blue squares
    lab_draw_command redSquare_cmd = {
        .type = LAB_DRAW_COMMAND_TRIANGLES,
        .triangles = {
            .vertices = redSquare,
            .vertexCount = 4
        }
    };
    
    lab_draw_command blueSquare_cmd = {
        .type = LAB_DRAW_COMMAND_TRIANGLES,
        .triangles = {
            .vertices = blueSquare,
            .vertexCount = 4
        }
    };
    
    commands.push_back(DrawCommand(redSquare_cmd));
    commands.push_back(DrawCommand(blueSquare_cmd));
    
    result = backend->SubmitCommands(commands);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    backend->EndFrame();
    
    // Read back and verify blending
    std::vector<uint8_t> readback(256 * 256 * 4);
    result = backend->ReadbackTexture(target->GetColorTexture(), readback.data(), readback.size());
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Verify we have some purple pixels (blended red and blue)
    bool hasBlendedPixels = false;
    for (size_t i = 0; i < readback.size(); i += 4) {
        if (readback[i] > 0 && readback[i + 2] > 0) {
            hasBlendedPixels = true;
            break;
        }
    }
    munit_assert_true(hasBlendedPixels);
    
    return MUNIT_OK;
}

static MunitTest wgpu_backend_tests[] = {
    {
        (char*)"/initialization",
        test_wgpu_initialization,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        (char*)"/texture_operations",
        test_wgpu_texture_operations,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        (char*)"/render_target",
        test_wgpu_render_target,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        (char*)"/blend_modes",
        test_wgpu_blend_modes,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

extern "C" MunitSuite wgpu_suite = {
    (char*)"/wgpu_backend",
    wgpu_backend_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
