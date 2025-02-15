# Download and configure munit
include(FetchContent)
FetchContent_Declare(
    munit
    GIT_REPOSITORY https://github.com/nemequ/munit.git
    GIT_TAG master
)
FetchContent_MakeAvailable(munit)

# Unit tests
add_executable(labfont_unit_tests)
target_sources(labfont_unit_tests
    PRIVATE
        unit/test_context.c
        # unit/test_font.c
        # unit/test_drawing.c
)

target_include_directories(labfont_unit_tests
    PRIVATE
        ${munit_SOURCE_DIR}
)

target_link_libraries(labfont_unit_tests
    PRIVATE
        labfont
        munit
)

# Visual tests
add_executable(labfont_visual_tests)
target_sources(labfont_visual_tests
    PRIVATE
        # Visual test files will be added here
        # visual/test_rendering.c
        # visual/test_text.c
)

target_link_libraries(labfont_visual_tests
    PRIVATE
        labfont
        munit
)

# Register tests
add_test(NAME unit_tests COMMAND labfont_unit_tests)
add_test(NAME visual_tests COMMAND labfont_visual_tests)
