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
    lab_result result = lab_create_context(LAB_BACKEND_METAL, &desc, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(ctx);

    // Test invalid parameters
    result = lab_create_context(LAB_BACKEND_METAL, NULL, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    result = lab_create_context(LAB_BACKEND_METAL, &desc, NULL);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    // Test cleanup
    lab_destroy_context(ctx);
    lab_destroy_context(NULL);  // Should handle null gracefully

    return MUNIT_OK;
}

static MunitResult test_context_resize(const MunitParameter params[], void* data) {
    lab_context ctx = NULL;
    lab_context_desc desc = {
        .width = 800,
        .height = 600,
        .native_window = NULL,
        .max_vertices = 1000,
        .atlas_width = 1024,
        .atlas_height = 1024
    };

    lab_result result = lab_create_context(LAB_BACKEND_METAL, &desc, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);

    // Test valid resize
    result = lab_resize_context(ctx, 1024, 768);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);

    // Test invalid parameters
    result = lab_resize_context(NULL, 1024, 768);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    result = lab_resize_context(ctx, 0, 768);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    result = lab_resize_context(ctx, 1024, 0);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    lab_destroy_context(ctx);
    return MUNIT_OK;
}

static MunitResult test_context_state(const MunitParameter params[], void* data) {
    lab_context ctx = NULL;
    lab_context_desc desc = {
        .width = 800,
        .height = 600,
        .native_window = NULL,
        .max_vertices = 1000,
        .atlas_width = 1024,
        .atlas_height = 1024
    };

    lab_result result = lab_create_context(LAB_BACKEND_METAL, &desc, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);

    // Test frame management
    lab_begin_frame(ctx);
    lab_clear(ctx, (lab_color){0.0f, 0.0f, 0.0f, 1.0f});
    lab_end_frame(ctx);

    // Test viewport
    lab_set_viewport(ctx, 0, 0, 800, 600);

    // Test null handling
    lab_begin_frame(NULL);  // Should handle null gracefully
    lab_end_frame(NULL);
    lab_clear(NULL, (lab_color){0.0f, 0.0f, 0.0f, 1.0f});
    lab_set_viewport(NULL, 0, 0, 800, 600);

    lab_destroy_context(ctx);
    return MUNIT_OK;
}

static MunitTest context_tests[] = {
    {
        "/creation",
        test_context_creation,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/resize",
        test_context_resize,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/state",
        test_context_state,
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
