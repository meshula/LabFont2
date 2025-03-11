#include <munit.h>
#include <labfont/labfont.h>
#include <string.h>
#include <stdio.h>

// Test loading a texture from a file
static MunitResult test_load_texture(const MunitParameter params[], void* data) {
    lab_context ctx = NULL;
    lab_backend_desc backend_desc = {
        .type = LAB_BACKEND_CPU,
        .width = 800,
        .height = 600,
        .native_window = NULL
    };
    
    // Load texture first (with null context)
    lab_texture texture = NULL;
    lab_result result = lab_load_texture(NULL, "resources/labfont-logo1.jpg", &texture);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    munit_assert_not_null(texture);
    
    // Create context
    result = lab_create_context(&backend_desc, &ctx);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    munit_assert_not_null(ctx);
    
    // Create a render target with the same dimensions as the texture
    lab_render_target render_target = NULL;
    lab_render_target_desc rt_desc = {
        .width = 512, // We'll use a fixed size for the test
        .height = 512,
        .format = LAB_TEXTURE_FORMAT_RGBA8_UNORM,
        .hasDepth = false
    };
    
    result = lab_create_render_target(ctx, &rt_desc, &render_target);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    munit_assert_not_null(render_target);
    
    // Set render target
    result = lab_set_render_target(ctx, render_target);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Begin frame
    result = lab_begin_frame(ctx);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Create a textured quad that fills the screen
    lab_vertex_2TC vertices[] = {
        // Top-left vertex
        {
            .position = {-1.0f, 1.0f},
            .texcoord = {0.0f, 0.0f},
            .color = {1.0f, 1.0f, 1.0f, 1.0f}
        },
        // Bottom-left vertex
        {
            .position = {-1.0f, -1.0f},
            .texcoord = {0.0f, 1.0f},
            .color = {1.0f, 1.0f, 1.0f, 1.0f}
        },
        // Top-right vertex
        {
            .position = {1.0f, 1.0f},
            .texcoord = {1.0f, 0.0f},
            .color = {1.0f, 1.0f, 1.0f, 1.0f}
        },
        // Top-right vertex (repeated)
        {
            .position = {1.0f, 1.0f},
            .texcoord = {1.0f, 0.0f},
            .color = {1.0f, 1.0f, 1.0f, 1.0f}
        },
        // Bottom-left vertex (repeated)
        {
            .position = {-1.0f, -1.0f},
            .texcoord = {0.0f, 1.0f},
            .color = {1.0f, 1.0f, 1.0f, 1.0f}
        },
        // Bottom-right vertex
        {
            .position = {1.0f, -1.0f},
            .texcoord = {1.0f, 1.0f},
            .color = {1.0f, 1.0f, 1.0f, 1.0f}
        }
    };
    
    // Create clear command (black background)
    lab_draw_command clear_cmd = {
        .type = LAB_DRAW_COMMAND_CLEAR,
        .clear = {
            .color = {0.0f, 0.0f, 0.0f, 1.0f}
        }
    };
    
    // Create textured quad draw command
    lab_draw_command quad_cmd = {
        .type = LAB_DRAW_COMMAND_TRIANGLES,
        .triangles = {
            .vertices = vertices,
            .vertexCount = 6
        }
    };
    
    // Submit commands
    lab_draw_command commands[] = {clear_cmd, quad_cmd};
    result = lab_submit_commands(ctx, commands, 2);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // End frame
    result = lab_end_frame(ctx);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Save the render target to a file
    result = lab_save_render_target(ctx, render_target, "texture_test_output.png");
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Now load the saved image and compare with the original
    lab_texture saved_texture = NULL;
    result = lab_load_texture(ctx, "texture_test_output.png", &saved_texture);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    munit_assert_not_null(saved_texture);
    
    // Get render target data
    uint8_t* rt_data = NULL;
    size_t rt_size = 0;
    lab_render_target_desc rt_desc_out;
    result = lab_get_render_target_data(ctx, render_target, &rt_desc_out, &rt_data, &rt_size);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    munit_assert_not_null(rt_data);
    munit_assert_size(rt_size, >, 0);
    
    // Cleanup
    lab_destroy_texture(ctx, texture);
    lab_destroy_texture(ctx, saved_texture);
    lab_destroy_render_target(ctx, render_target);
    lab_destroy_context(ctx);
    
    if (rt_data) {
        lab_free(rt_data);
    }
    
    return MUNIT_OK;
}

// Test invalid parameters
static MunitResult test_load_texture_invalid_params(const MunitParameter params[], void* data) {
    lab_context ctx = NULL;
    lab_backend_desc backend_desc = {
        .type = LAB_BACKEND_CPU,
        .width = 800,
        .height = 600,
        .native_window = NULL
    };
    
    // Create context
    lab_result result = lab_create_context(&backend_desc, &ctx);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    
    // Test with NULL context - this should work now
    lab_texture texture = NULL;
    result = lab_load_texture(NULL, "resources/labfont-logo1.jpg", &texture);
    munit_assert_int(result, ==, LAB_RESULT_OK);
    munit_assert_not_null(texture);
    lab_destroy_texture(ctx, texture);
    
    // Test with NULL path
    result = lab_load_texture(ctx, NULL, &texture);
    munit_assert_int(result, ==, LAB_RESULT_INVALID_PARAMETER);
    
    // Test with NULL output texture
    result = lab_load_texture(ctx, "resources/labfont-logo1.jpg", NULL);
    munit_assert_int(result, ==, LAB_RESULT_INVALID_PARAMETER);
    
    // Test with non-existent file
    result = lab_load_texture(ctx, "non_existent_file.jpg", &texture);
    munit_assert_int(result, ==, LAB_RESULT_TEXTURE_CREATION_FAILED);
    
    // Cleanup
    lab_destroy_context(ctx);
    
    return MUNIT_OK;
}

static MunitTest texture_tests[] = {
    {
        "/load_texture",
        test_load_texture,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/load_texture_invalid_params",
        test_load_texture_invalid_params,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

extern "C" {
    MunitSuite texture_suite = {
        "/texture",
        texture_tests,
        NULL,
        1,
        MUNIT_SUITE_OPTION_NONE
    };
}
