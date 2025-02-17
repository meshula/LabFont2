# Test suite CMake configuration
cmake_minimum_required(VERSION 3.15)

# Add munit library
add_library(munit STATIC
    ${CMAKE_SOURCE_DIR}/third_party/munit/munit.c
)
target_include_directories(munit PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/munit
)

# Unit tests executable
add_executable(labfont_tests
    unit/test_main.cpp
    unit/test_context.c
    unit/test_resources.c
    unit/test_error.c
    unit/test_memory.c
    unit/test_backend.cpp
)

# Add backend-specific tests based on enabled backends
if(LABFONT_ENABLE_VULKAN)
    target_sources(labfont_tests PRIVATE
        unit/test_vulkan_backend.cpp
    )
    target_compile_definitions(labfont_tests PRIVATE LABFONT_VULKAN_ENABLED=1)
endif()

if(LABFONT_ENABLE_METAL)
    target_sources(labfont_tests PRIVATE
        unit/test_metal_backend.cpp
    )
    target_compile_definitions(labfont_tests PRIVATE LABFONT_METAL_ENABLED=1)
endif()

if(LABFONT_ENABLE_WGPU)
    target_sources(labfont_tests PRIVATE
        unit/test_wgpu_backend.cpp
    )
    target_compile_definitions(labfont_tests PRIVATE LABFONT_WGPU_ENABLED=1)
endif()

if(LABFONT_ENABLE_DX11)
    target_sources(labfont_tests PRIVATE
        unit/test_dx11_backend.cpp
    )
    target_compile_definitions(labfont_tests PRIVATE LABFONT_DX11_ENABLED=1)
endif()

# Include directories
target_include_directories(labfont_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Link dependencies
target_link_libraries(labfont_tests PRIVATE
    labfont
    munit
)

# Backend-specific dependencies
if(LABFONT_ENABLE_VULKAN)
    target_link_libraries(labfont_tests PRIVATE
        Vulkan::Vulkan
    )
endif()

if(LABFONT_ENABLE_METAL)
    target_link_libraries(labfont_tests PRIVATE
        "-framework Metal"
        "-framework QuartzCore"
        "-framework Foundation"
    )
    
    # Copy Metal shader library to test executable directory
    add_custom_command(TARGET labfont_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/src/backends/metal/shaders/compiled/primitives.metallib
            $<TARGET_FILE_DIR:labfont_tests>/primitives.metallib
        COMMENT "Copying Metal shader library to test executable directory"
    )
endif()

# Set C++ standard
set_target_properties(labfont_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Add test
add_test(NAME labfont_tests COMMAND labfont_tests)
