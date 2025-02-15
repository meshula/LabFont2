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
        # src/core/context.cpp
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
if(LABFONT_ENABLE_METAL)
    # target_sources(labfont PRIVATE src/backends/metal/...)
endif()

if(LABFONT_ENABLE_WGPU)
    # target_sources(labfont PRIVATE src/backends/wgpu/...)
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
