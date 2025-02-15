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
    lab_result result = lab_create_context(LAB_BACKEND_METAL, &desc, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(ctx);

    // Create texture
    lab_texture tex = NULL;
    lab_texture_desc tex_desc = {
        .width = 256,
        .height = 256,
        .format = 1  // Assuming 1 is RGBA8
    };
    result = lab_create_texture(ctx, "test_texture", &tex_desc, &tex);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(tex);

    // Create buffer
    lab_buffer buf = NULL;
    lab_buffer_desc buf_desc = {
        .size = 1024,
        .dynamic = true
    };
    result = lab_create_buffer(ctx, "test_buffer", &buf_desc, &buf);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);
    munit_assert_not_null(buf);

    // Test invalid parameters
    result = lab_create_texture(NULL, "test_texture", &tex_desc, &tex);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    result = lab_create_texture(ctx, NULL, &tex_desc, &tex);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    result = lab_create_texture(ctx, "test_texture", NULL, &tex);
    munit_assert_int(result.error, ==, LAB_ERROR_INVALID_PARAMETER);

    // Cleanup
    lab_destroy_texture(ctx, tex);
    lab_destroy_buffer(ctx, buf);
    lab_destroy_context(ctx);

    return MUNIT_OK;
}

static MunitResult test_resource_retrieval(const MunitParameter params[], void* data) {
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
    lab_result result = lab_create_context(LAB_BACKEND_METAL, &desc, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);

    // Create and retrieve texture
    lab_texture tex = NULL;
    lab_texture_desc tex_desc = {
        .width = 256,
        .height = 256,
        .format = 1
    };
    result = lab_create_texture(ctx, "test_texture", &tex_desc, &tex);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);

    lab_texture retrieved_tex = lab_get_texture(ctx, "test_texture");
    munit_assert_ptr_equal(tex, retrieved_tex);

    // Test non-existent resource
    lab_texture missing_tex = lab_get_texture(ctx, "nonexistent");
    munit_assert_null(missing_tex);

    // Cleanup
    lab_destroy_texture(ctx, tex);
    lab_destroy_context(ctx);

    return MUNIT_OK;
}

static MunitResult test_resource_destruction(const MunitParameter params[], void* data) {
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
    lab_result result = lab_create_context(LAB_BACKEND_METAL, &desc, &ctx);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);

    // Create and destroy texture
    lab_texture tex = NULL;
    lab_texture_desc tex_desc = {
        .width = 256,
        .height = 256,
        .format = 1
    };
    result = lab_create_texture(ctx, "test_texture", &tex_desc, &tex);
    munit_assert_int(result.error, ==, LAB_ERROR_NONE);

    lab_destroy_texture(ctx, tex);
    
    // Verify texture is no longer retrievable
    lab_texture retrieved_tex = lab_get_texture(ctx, "test_texture");
    munit_assert_null(retrieved_tex);

    // Test destroying null/invalid resources
    lab_destroy_texture(ctx, NULL);  // Should handle gracefully
    lab_destroy_texture(NULL, tex);  // Should handle gracefully

    lab_destroy_context(ctx);

    return MUNIT_OK;
}

static MunitTest resource_tests[] = {
    {
        "/creation",
        test_resource_creation,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/retrieval",
        test_resource_retrieval,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/destruction",
        test_resource_destruction,
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
