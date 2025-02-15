cmake_minimum_required(VERSION 3.15)
project(LabFont 
    VERSION 2.0.0
    DESCRIPTION "Modern text rendering and immediate mode drawing library"
    LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(LABFONT_BUILD_TESTS "Build tests" ON)
option(LABFONT_BUILD_EXAMPLES "Build examples" ON)

# Backend options
option(LABFONT_ENABLE_METAL "Enable Metal backend" OFF)
option(LABFONT_ENABLE_WGPU "Enable WebGPU backend" OFF)
option(LABFONT_ENABLE_VULKAN "Enable Vulkan backend" OFF)
option(LABFONT_ENABLE_DX11 "Enable DirectX 11 backend" OFF)

# WebGPU dependency
if(LABFONT_ENABLE_WGPU)
    include(FetchContent)
    FetchContent_Declare(
        webgpu
        GIT_REPOSITORY https://github.com/webgpu-native/webgpu-headers.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(webgpu)
    
    FetchContent_Declare(
        wgpu
        GIT_REPOSITORY https://github.com/gfx-rs/wgpu-native.git
        GIT_TAG trunk
    )
    FetchContent_MakeAvailable(wgpu)
endif()

# Library target
add_library(labfont)
target_include_directories(labfont
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb
)

# Source files will be added here as they are created
target_sources(labfont
    PRIVATE
        # Core files
        src/core/context.cpp
        src/core/resource_manager.cpp
        src/core/error.cpp
        src/core/memory.cpp
        # src/core/font.cpp
        # src/core/atlas.cpp
        
        # Text rendering
        # src/text/shaping.cpp
        # src/text/layout.cpp
        
        # Drawing
        # src/draw/primitives.cpp
        # src/draw/path.cpp
)

# Backend-specific source files
# CPU backend is always enabled as it's used for testing
target_sources(labfont
    PRIVATE
        src/backends/cpu/cpu_backend.h
        src/backends/cpu/cpu_backend.cpp
)

if(LABFONT_ENABLE_METAL)
    # Add Metal backend sources
    target_sources(labfont
        PRIVATE
            src/backends/metal/metal_backend.h
            src/backends/metal/metal_backend.mm
            src/backends/metal/metal_command_buffer.h
            src/backends/metal/metal_command_buffer.mm
            src/backends/metal/metal_types.h
    )
    
    # Compile Metal shaders
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/metal/shaders/compiled/primitives.metallib
        COMMAND
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/metal/shaders/compile_shaders.sh
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/metal/shaders/primitives.metal
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/metal/shaders/compile_shaders.sh
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/metal/shaders
        COMMENT
            "Compiling Metal shaders"
        VERBATIM
    )
    
    # Add shader compilation to build dependencies
    add_custom_target(metal_shaders
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/metal/shaders/compiled/primitives.metallib
    )
    add_dependencies(labfont metal_shaders)
    
    # Link Metal framework
    target_link_libraries(labfont
        PRIVATE
            "-framework Metal"
            "-framework QuartzCore"
            "-framework Foundation"
    )
    
    # Enable Metal backend
    target_compile_definitions(labfont
        PRIVATE
            LABFONT_METAL_ENABLED=1
    )
endif()

if(LABFONT_ENABLE_WGPU)
    target_sources(labfont
        PRIVATE
            src/backends/wgpu/wgpu_types.h
            src/backends/wgpu/wgpu_backend.h
            src/backends/wgpu/wgpu_backend.cpp
            src/backends/wgpu/wgpu_command_buffer.h
            src/backends/wgpu/wgpu_command_buffer.cpp
    )
    
    target_include_directories(labfont
        PRIVATE
            ${webgpu_SOURCE_DIR}/include
    )
    
    target_link_libraries(labfont
        PRIVATE
            wgpu_native
    )
    
    target_compile_definitions(labfont
        PRIVATE
            LABFONT_WGPU_ENABLED=1
    )
endif()

if(LABFONT_ENABLE_VULKAN)
    # target_sources(labfont PRIVATE src/backends/vulkan/...)
endif()

if(LABFONT_ENABLE_DX11)
    # target_sources(labfont PRIVATE src/backends/dx11/...)
endif()

# Tests
if(LABFONT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(LABFONT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
include(GNUInstallDirs)
install(TARGETS labfont
    EXPORT labfont-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/labfont
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Export targets
install(EXPORT labfont-targets
    FILE labfont-targets.cmake
    NAMESPACE LabFont::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/labfont
)
