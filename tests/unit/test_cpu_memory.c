#include <munit.h>
#include <labfont/labfont.h>
#include <string.h>

static MunitResult test_basic_allocation(const MunitParameter params[], void* data) {
    // Test allocation
    void* ptr = lab_alloc(1024, LAB_MEMORY_GENERAL);
    munit_assert_not_null(ptr);

    // Write to memory to ensure it's usable
    memset(ptr, 0xAA, 1024);

    // Free memory
    lab_free(ptr);

    // Test null pointer handling
    lab_free(NULL);  // Should not crash

    return MUNIT_OK;
}

static MunitResult test_memory_tracking(const MunitParameter params[], void* data) {
    lab_context ctx = NULL;
    lab_context_desc desc = {
        .width = 800,
        .height = 600,
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
    lab_result result = lab_create_context(&backend_desc, &ctx);
    munit_assert_int(result, ==, LAB_RESULT_OK);

    // Get initial stats
    lab_memory_stats stats = lab_get_memory_stats();
    size_t initial_usage = stats.currentUsage;

    // Allocate some memory
    void* ptr1 = lab_alloc(1024, LAB_MEMORY_GENERAL);
    void* ptr2 = lab_alloc(2048, LAB_MEMORY_GRAPHICS);
    munit_assert_not_null(ptr1);
    munit_assert_not_null(ptr2);

    // Check updated stats
    stats = lab_get_memory_stats();
    munit_assert_size(stats.currentUsage, ==, initial_usage + 3072);
    munit_assert_size(stats.totalAllocated, >=, 3072);

    // Free memory
    lab_free(ptr1);
    stats = lab_get_memory_stats();
    munit_assert_size(stats.currentUsage, ==, initial_usage + 2048);

    lab_free(ptr2);
    stats = lab_get_memory_stats();
    munit_assert_size(stats.currentUsage, ==, initial_usage);

    // Reset memory stats
    lab_reset_memory_stats();
    stats = lab_get_memory_stats();
    munit_assert_size(stats.totalAllocated, ==, 0);
    munit_assert_size(stats.totalFreed, ==, 0);
    munit_assert_size(stats.currentUsage, ==, 0);
    munit_assert_size(stats.peakUsage, ==, 0);

    lab_destroy_context(ctx);
    return MUNIT_OK;
}

static MunitResult test_memory_categories(const MunitParameter params[], void* data) {
    lab_context ctx = NULL;
    lab_context_desc desc = {
        .width = 800,
        .height = 600,
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
    lab_result result = lab_create_context(&backend_desc, &ctx);
    munit_assert_int(result, ==, LAB_RESULT_OK);

    // Allocate memory in different categories
    void* ptr1 = lab_alloc(1024, LAB_MEMORY_GENERAL);
    void* ptr2 = lab_alloc(2048, LAB_MEMORY_GRAPHICS);
    void* ptr3 = lab_alloc(512, LAB_MEMORY_TEXT);
    munit_assert_not_null(ptr1);
    munit_assert_not_null(ptr2);
    munit_assert_not_null(ptr3);

    // Check category usage
    lab_memory_stats stats = lab_get_memory_stats();
    munit_assert_size(stats.categoryUsage[LAB_MEMORY_GENERAL], ==, 1024);
    munit_assert_size(stats.categoryUsage[LAB_MEMORY_GRAPHICS], ==, 2048);
    munit_assert_size(stats.categoryUsage[LAB_MEMORY_TEXT], ==, 512);

    // Free memory
    lab_free(ptr1);
    lab_free(ptr2);
    lab_free(ptr3);

    // Reset memory stats
    lab_reset_memory_stats();
    stats = lab_get_memory_stats();
    munit_assert_size(stats.categoryUsage[LAB_MEMORY_GENERAL], ==, 0);
    munit_assert_size(stats.categoryUsage[LAB_MEMORY_GRAPHICS], ==, 0);
    munit_assert_size(stats.categoryUsage[LAB_MEMORY_TEXT], ==, 0);

    lab_destroy_context(ctx);
    return MUNIT_OK;
}

static MunitTest memory_tests[] = {
    {
        "/basic_allocation",
        test_basic_allocation,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/tracking",
        test_memory_tracking,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    {
        "/categories",
        test_memory_categories,
        NULL,
        NULL,
        MUNIT_TEST_OPTION_NONE,
        NULL
    },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

const MunitSuite memory_suite = {
    "/memory",
    memory_tests,
    NULL,
    1,
    MUNIT_SUITE_OPTION_NONE
};
