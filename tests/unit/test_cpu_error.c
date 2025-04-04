#include <munit.h>
#include <labfont/labfont.h>
#include <string.h>

static MunitResult test_error_strings(const MunitParameter params[], void* data) {
    // Test valid error codes
    const char* none_str = lab_get_error_string(LAB_RESULT_OK);
    munit_assert_not_null(none_str);
    munit_assert_string_equal(none_str, "No error");

    const char* invalid_param_str = lab_get_error_string(LAB_RESULT_INVALID_PARAMETER);
    munit_assert_not_null(invalid_param_str);
    munit_assert_string_equal(invalid_param_str, "Invalid parameter");

    const char* oom_str = lab_get_error_string(LAB_RESULT_OUT_OF_MEMORY);
    munit_assert_not_null(oom_str);
    munit_assert_string_equal(oom_str, "Out of memory");

    // Test invalid error code
    const char* unknown_str = lab_get_error_string((lab_result)999);
    munit_assert_not_null(unknown_str);
    munit_assert_string_equal(unknown_str, "Unknown error");

    return MUNIT_OK;
}

static MunitResult test_error_propagation(const MunitParameter params[], void* data) {
    // Test error propagation through context creation
    lab_context ctx = NULL;
    lab_context_desc desc = {0};  // Invalid desc (width and height are 0)
    
    lab_backend_desc backend_desc = {
        .type = LAB_BACKEND_CPU,
        .width = desc.width,
        .height = desc.height,
        .native_window = desc.native_window
    };
    lab_result result = lab_create_context(&backend_desc, &ctx);
    munit_assert_int(result, ==, LAB_RESULT_INVALID_DIMENSION);
    
    // Test error propagation through resource creation with invalid dimensions
    lab_texture tex = NULL;
    lab_texture_desc tex_desc = {0};  // Invalid desc (width and height are 0)
    
    // Even with NULL context, invalid dimensions should still fail
    result = lab_create_texture(NULL, &tex_desc, &tex);
    munit_assert_int(result, ==, LAB_RESULT_INVALID_DIMENSION);

    return MUNIT_OK;
}

static MunitResult test_error_clearing(const MunitParameter params[], void* data) {
    // Create a valid context
    lab_context ctx = NULL;
    lab_context_desc desc = {
        .width = 800,
        .height = 600,
        .max_vertices = 1000,
        .atlas_width = 1024,
        .atlas_height = 1024
    };
    
    // First operation should succeed
    lab_backend_desc backend_desc = {
        .type = LAB_BACKEND_CPU,
        .width = desc.width,
        .height = desc.height,
        .native_window = desc.native_window
    };
    lab_result result = lab_create_context(&backend_desc, &ctx);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Failed operation
    lab_texture tex = NULL;
    lab_texture_desc tex_desc = {0};  // Invalid desc
    result = lab_create_texture(ctx, &tex_desc, &tex);
    munit_assert_int(result, ==, LAB_RESULT_INVALID_DIMENSION);
    
    // Next valid operation should succeed with no error
    lab_texture_desc valid_desc = {
        .width = 256,
        .height = 256,
        .format = 1
    };
    result = lab_create_texture(ctx, &valid_desc, &tex);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Cleanup
    lab_destroy_texture(ctx, tex);
    lab_destroy_context(ctx);

    return MUNIT_OK;
}

static MunitTest error_tests[] = {
    {
        "/strings",
        test_error_strings,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/propagation",
        test_error_propagation,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/clearing",
        test_error_clearing,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

const MunitSuite error_suite = {
    "/error",
    error_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
