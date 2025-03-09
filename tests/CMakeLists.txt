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
    unit/test_cpu_context.c
    unit/test_cpu_resources.c
    unit/test_cpu_error.c
    unit/test_cpu_memory.c
    unit/test_cpu_backend.cpp
    unit/test_texture_loading.cpp
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
    
    # Add Emscripten WebGPU include paths for tests
    if(EMSCRIPTEN)
        target_include_directories(labfont_tests
            PRIVATE
                ${EMSCRIPTEN_INCLUDE_DIR}
                ${EMSCRIPTEN_INCLUDE_DIR}/webgpu
                ${EMSCRIPTEN_INCLUDE_DIR}/emscripten
        )
        target_compile_definitions(labfont_tests
            PRIVATE
                __EMSCRIPTEN__
                WEBGPU_CPP_IMPLEMENTATION
        )
    endif()
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

# WebGPU browser tests when building with Emscripten
if(EMSCRIPTEN)
    add_executable(labfont_wgpu_tests "web/test_wgpu_main.cpp")
    target_link_libraries(labfont_wgpu_tests
        PRIVATE
            labfont
            munit
    )
    
    # Set C++20 standard for WebGPU support
    set_target_properties(labfont_wgpu_tests PROPERTIES CXX_STANDARD 20)
    
    # Configure Emscripten build
    set_target_properties(labfont_wgpu_tests PROPERTIES SUFFIX ".html")
    target_link_options(labfont_wgpu_tests PRIVATE "-sUSE_WEBGPU=1" "-sUSE_GLFW=3")
    
    # Copy HTML template and ensure test_runner.js is in the right place
    add_custom_command(TARGET labfont_wgpu_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/web/test.html
            $<TARGET_FILE_DIR:labfont_wgpu_tests>/labfont_wgpu_tests.html
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/web/test_runner.js
            $<TARGET_FILE_DIR:labfont_wgpu_tests>/test_runner.js
        COMMENT "Copying web test files"
    )
endif()

# Copy resources folder to build directory for tests
add_custom_command(TARGET labfont_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:labfont_tests>/resources
    COMMENT "Copying resources folder to tests build directory"
)

# Add test
add_test(NAME labfont_tests COMMAND labfont_tests)

add_executable(labfont_style_parser_tests
    unit/test_labfont_style_parser.cpp
)
target_link_libraries(labfont_style_parser_tests
PRIVATE
    labfont
)
