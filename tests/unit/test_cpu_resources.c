#include <munit.h>
#include <labfont/labfont.h>

static MunitResult test_resource_creation(const MunitParameter params[], void* data) {
    lab_context ctx = NULL;
    lab_context_desc desc = {
        .width = 800,
        .height = 600,
        .native_window = NULL,
        .max_vertices = 1000,
        .atlas_width = 1024,
        .atlas_height = 1024
    };

    // Create context
    lab_backend_desc backend_desc = {
        .type = LAB_BACKEND_CPU,
        .width = desc.width,
        .height = desc.height,
        .native_window = desc.native_window
    };
    lab_operation_result result = lab_create_context(&backend_desc, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(ctx);

    // Create texture
    lab_texture tex = NULL;
    lab_texture_desc tex_desc = {
        .width = 256,
        .height = 256,
        .format = 1  // Assuming 1 is RGBA8
    };
    result = lab_create_texture(ctx, &tex_desc, &tex);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(tex);

    // Create buffer
    lab_buffer buf = NULL;
    lab_buffer_desc buf_desc = {
        .size = 1024,
        .dynamic = true
    };
    result = lab_create_buffer(ctx, &buf_desc, &buf);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(buf);

    // Test invalid parameters
    result = lab_create_texture(NULL, &tex_desc, &tex);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    // The test for null name parameter is removed as there's no name parameter in the API

    result = lab_create_texture(ctx, NULL, &tex);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    // Cleanup
    lab_destroy_texture(ctx, tex);
    lab_destroy_buffer(ctx, buf);
    lab_destroy_context(ctx);

    return MUNIT_OK;
}

// Note: The resource retrieval and destruction tests are removed because they use
// lab_get_texture which doesn't exist in the current API

static MunitTest resource_tests[] = {
    {
        "/creation",
        test_resource_creation,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

const MunitSuite resource_suite = {
    "/resource",
    resource_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
