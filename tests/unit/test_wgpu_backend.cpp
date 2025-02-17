#include <munit.h>
#include <labfont/labfont.h>
#include "../../src/backends/wgpu/wgpu_backend.h"
#include "../utils/test_patterns.h"

using namespace labfont;
using namespace labfont::wgpu;
using namespace labfont::test;

static MunitResult test_wgpu_initialization(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<WGPUBackend>();
    
    // Initialize backend
    lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Verify supported features
    munit_assert_true(backend->SupportsTextureFormat(TextureFormat::RGBA8_UNORM));
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
        .format = TextureFormat::RGBA8_UNORM,
        .renderTarget = false,
        .readback = true,
        .data = nullptr
    };
    
    std::shared_ptr<Texture> texture;
    lab_result result = backend->CreateTexture(desc, texture);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
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
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    // Read back and verify
    std::vector<uint8_t> readback(pattern.size());
    result = backend->ReadbackTexture(texture.get(), readback.data(), readback.size());
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
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
        .format = TextureFormat::RGBA8_UNORM,
        .hasDepth = true
    };
    
    std::shared_ptr<RenderTarget> target;
    lab_result result = backend->CreateRenderTarget(desc, target);
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
    
    // Read back render target
    std::vector<uint8_t> readback(512 * 512 * 4);
    result = backend->ReadbackTexture(target->GetColorTexture(), readback.data(), readback.size());
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
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
        .format = TextureFormat::RGBA8_UNORM,
        .hasDepth = false
    };
    
    std::shared_ptr<RenderTarget> target;
    lab_result result = backend->CreateRenderTarget(desc, target);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    backend->SetRenderTarget(target.get());
    backend->BeginFrame();
    
    // Test vertices for overlapping squares
    std::vector<Vertex> redSquare = {
        {{-0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.5f}},
        {{ 0.0f, -0.5f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.5f}},
        {{-0.5f,  0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 0.5f}},
        {{ 0.0f,  0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 0.5f}}
    };
    
    std::vector<Vertex> blueSquare = {
        {{-0.25f, -0.25f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.5f}},
        {{ 0.25f, -0.25f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.5f}},
        {{-0.25f,  0.25f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.5f}},
        {{ 0.25f,  0.25f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.5f}}
    };
    
    std::vector<DrawCommand> commands;
    
    // Clear command
    DrawCommand clear;
    clear.type = DrawCommandType::Clear;
    clear.clear.color[0] = 0.0f;
    clear.clear.color[1] = 0.0f;
    clear.clear.color[2] = 0.0f;
    clear.clear.color[3] = 1.0f;
    commands.push_back(clear);
    
    // Set alpha blend mode
    DrawCommand blend;
    blend.type = DrawCommandType::SetBlendMode;
    blend.blend.mode = BlendMode::Alpha;
    commands.push_back(blend);
    
    // Draw red square
    DrawCommand drawRed;
    drawRed.type = DrawCommandType::DrawTriangles;
    drawRed.triangles.vertices = redSquare.data();
    drawRed.triangles.vertexCount = redSquare.size();
    commands.push_back(drawRed);
    
    // Draw blue square
    DrawCommand drawBlue;
    drawBlue.type = DrawCommandType::DrawTriangles;
    drawBlue.triangles.vertices = blueSquare.data();
    drawBlue.triangles.vertexCount = blueSquare.size();
    commands.push_back(drawBlue);
    
    result = backend->SubmitCommands(commands);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
    backend->EndFrame();
    
    // Read back and verify blending
    std::vector<uint8_t> readback(256 * 256 * 4);
    result = backend->ReadbackTexture(target->GetColorTexture(), readback.data(), readback.size());
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
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

extern "C" MunitSuite wgpu_backend_suite = {
    (char*)"/wgpu_backend",
    wgpu_backend_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
