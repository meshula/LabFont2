#include <munit.h>

/* Forward declarations of test suites */
extern "C" {
    // CPU backend test suites
    extern MunitSuite context_suite;
    extern MunitSuite resource_suite;
    extern MunitSuite error_suite;
    extern MunitSuite memory_suite;
    extern MunitSuite backend_suite;
    
    // Other backend test suites
    #ifdef LABFONT_VULKAN_ENABLED
    extern MunitSuite vulkan_suite;
    #endif
    
    #ifdef LABFONT_METAL_ENABLED
    extern MunitSuite metal_suite;
    #endif
    
    #ifdef LABFONT_WGPU_ENABLED
    extern MunitSuite wgpu_suite;
    #endif
    
    #ifdef LABFONT_DX11_ENABLED
    extern MunitSuite dx11_suite;
    #endif
}

static const char* suite_name = "/labfont";

int main(int argc, char* argv[]) {
    // Count number of enabled suites
    int suite_count = 5; // Core suites
    
    #ifdef LABFONT_VULKAN_ENABLED
    suite_count++;
    #endif
    
    #ifdef LABFONT_METAL_ENABLED
    suite_count++;
    #endif
    
    #ifdef LABFONT_WGPU_ENABLED
    suite_count++;
    #endif
    
    #ifdef LABFONT_DX11_ENABLED
    suite_count++;
    #endif
    
    // Allocate array for suites
    MunitSuite* suites = new MunitSuite[suite_count + 1]; // +1 for NULL terminator
    int idx = 0;
    
    // Add core suites
    suites[idx++] = context_suite;
    suites[idx++] = resource_suite;
    suites[idx++] = error_suite;
    suites[idx++] = memory_suite;
    suites[idx++] = backend_suite;
    
    // Add backend-specific suites
    #ifdef LABFONT_VULKAN_ENABLED
    suites[idx++] = vulkan_suite;
    #endif
    
    #ifdef LABFONT_METAL_ENABLED
    suites[idx++] = metal_suite;
    #endif
    
    #ifdef LABFONT_WGPU_ENABLED
    suites[idx++] = wgpu_suite;
    #endif
    
    #ifdef LABFONT_DX11_ENABLED
    suites[idx++] = dx11_suite;
    #endif
    
    // Add NULL terminator
    suites[idx] = { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE };

    MunitSuite main_suite = {
        (char*)suite_name,
        NULL,
        suites,
        1,
        MUNIT_SUITE_OPTION_NONE
    };

    int result = munit_suite_main(&main_suite, NULL, argc, argv);
    
    delete[] suites;
    return result;
}
