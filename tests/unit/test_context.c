#include <munit.h>
#include <labfont/labfont.h>

static MunitResult test_context_creation(const MunitParameter params[], void* data) {
    lab_context ctx = NULL;
    lab_context_desc desc = {
        .width = 800,
        .height = 600,
        .native_window = NULL,
        .max_vertices = 1000,
        .atlas_width = 1024,
        .atlas_height = 1024
    };

    // Test valid creation
    lab_backend_desc backend_desc = {
        .type = LAB_BACKEND_CPU,
        .width = desc.width,
        .height = desc.height,
        .native_window = desc.native_window
    };
    lab_operation_result result = lab_create_context(&backend_desc, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(ctx);

    // Test invalid parameters
    result = lab_create_context(NULL, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    lab_backend_desc valid_desc = {
        .type = LAB_BACKEND_METAL,
        .width = desc.width,
        .height = desc.height,
        .native_window = desc.native_window
    };
    result = lab_create_context(&valid_desc, NULL);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    // Test cleanup
    lab_destroy_context(ctx);
    lab_destroy_context(NULL);  // Should handle null gracefully

    return MUNIT_OK;
}

// Note: The resize and state tests are removed because the functions they test
// (lab_resize_context, lab_clear, lab_set_viewport) don't exist in the current API

static MunitTest context_tests[] = {
    {
        "/creation",
        test_context_creation,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

const MunitSuite context_suite = {
    "/context",
    context_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
