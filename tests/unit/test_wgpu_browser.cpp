#include <munit.h>
#include <labfont/labfont.h>
#include "../../src/backends/wgpu/wgpu_backend.h"
#include "../utils/test_patterns.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5_webgpu.h>
#endif

using namespace labfont;
using namespace labfont::wgpu;
using namespace labfont::test;

// Test results structure that will be exposed to JavaScript
struct TestResult {
    const char* name;
    bool passed;
    const char* message;
};

std::vector<TestResult> g_testResults;

static MunitResult test_wgpu_initialization(const MunitParameter params[], void* data) {
    auto backend = std::make_unique<WGPUBackend>();
    
    // Initialize backend
    lab_result result = backend->Initialize(800, 600);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    
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
    
    return MUNIT_OK;
}

static MunitTest wgpu_browser_tests[] = {
    {
        (char*)"/initialization",
        test_wgpu_initialization,
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
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

// Function to run tests and store results
extern "C" EMSCRIPTEN_KEEPALIVE int run_tests() {
    g_testResults.clear();
    
    for (const MunitTest* test = wgpu_browser_tests; test->test_name != NULL; ++test) {
        MunitResult result = test->test(nullptr, nullptr);
        
        g_testResults.push_back({
            test->test_name,
            result == MUNIT_OK,
            result == MUNIT_OK ? "Test passed" : "Test failed"
        });
    }
    
    // Return 0 if all tests passed, non-zero otherwise
    return std::any_of(g_testResults.begin(), g_testResults.end(),
                      [](const TestResult& r) { return !r.passed; }) ? 1 : 0;
}

// Function to get test results from JavaScript
extern "C" EMSCRIPTEN_KEEPALIVE const TestResult* get_test_results(int* count) {
    *count = g_testResults.size();
    return g_testResults.data();
}

// Main suite
extern "C" MunitSuite wgpu_browser_suite = {
    (char*)"/wgpu_browser",
    wgpu_browser_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
