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
option(LABFONT_BUILD_TESTS    "Build tests" ON)
option(LABFONT_BUILD_EXAMPLES "Build examples" ON)

# Backend options
option(LABFONT_ENABLE_METAL  "Enable Metal backend" OFF)
option(LABFONT_ENABLE_WGPU   "Enable WebGPU backend" OFF)
option(LABFONT_ENABLE_VULKAN "Enable Vulkan backend" OFF)
option(LABFONT_ENABLE_DX11   "Enable DirectX 11 backend" OFF)

# Vulkan dependency
if(LABFONT_ENABLE_VULKAN AND NOT EMSCRIPTEN)
    # Find Vulkan SDK
    find_package(Vulkan REQUIRED)
    
    if(APPLE)
        # Check for MoltenVK
        if(NOT EXISTS "$ENV{VULKAN_SDK}/lib/libMoltenVK.dylib")
            message(FATAL_ERROR "MoltenVK not found. Please install the LunarG Vulkan SDK.")
        endif()
        
        # Add MoltenVK to library path and rpath
        set(MOLTENVK_LIB "$ENV{VULKAN_SDK}/lib/libMoltenVK.dylib")
        set(CMAKE_INSTALL_RPATH "$ENV{VULKAN_SDK}/lib")
        set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    endif()
endif()

# Library target
add_library(labfont)

# WebAssembly/Emscripten configuration
if(EMSCRIPTEN)
    if(LABFONT_ENABLE_WGPU)
        # Enable WebGPU in Emscripten
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sUSE_WEBGPU=1")
        set(CMAKE_EXECUTABLE_SUFFIX ".html")
        
        # Add Emscripten WebGPU include paths
        set(EMSCRIPTEN_INCLUDE_DIR "${EMSCRIPTEN_ROOT}/cache/sysroot/include")
        target_include_directories(labfont
            PRIVATE
                ${EMSCRIPTEN_INCLUDE_DIR}
                ${EMSCRIPTEN_INCLUDE_DIR}/webgpu
                ${EMSCRIPTEN_INCLUDE_DIR}/emscripten
        )

        # Set Emscripten compile flags
        set(EMSCRIPTEN_FLAGS "-s USE_WEBGPU=1 -s WASM=1 -s ASSERTIONS=1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_FLAGS}")

        # Set WebGPU compile definitions
        target_compile_definitions(labfont
            PRIVATE
                __EMSCRIPTEN__
                WEBGPU_CPP_IMPLEMENTATION
        )
        
        # Set WebGPU compile definitions
        target_compile_definitions(labfont
            PRIVATE
                __EMSCRIPTEN__
        )
    endif()
endif()
target_include_directories(labfont
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/labfont
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb
)

set(CORE_INCLUDES
    include/labfont/labfont.h
    include/labfont/labfont_types.h
    include/labfont/labfont_renderer.h
    include/labfont/labfont_rich.h
    include/labfont/labfont_style_parser.h
)

set(CORE_SRC
    src/core/backend_types.h
    src/core/backend.h
    src/core/context_internal.h
    src/core/context.cpp
    src/core/draw_state.h
    src/core/error.cpp
    src/core/error.h
    src/core/font_manager.h
    src/core/internal_types.h
    #src/core/labfont_renderer.c
    src/core/memory.cpp
    src/core/memory.h
    src/core/resource_manager.cpp
    src/core/resource_manager.h
    src/core/resource.h
)

set(CPU_BACKEND_SRC
    src/backends/cpu/cpu_backend.h
    src/backends/cpu/cpu_backend.cpp
    src/backends/cpu/rasterizer.h
)

if(LABFONT_ENABLE_METAL AND NOT EMSCRIPTEN)
    # Add Metal backend sources
    set(METAL_BACKEND_SRC
        src/backends/metal/metal_backend.h
        src/backends/metal/metal_backend.mm
        src/backends/metal/metal_command_buffer.h
        src/backends/metal/metal_command_buffer.mm
        src/backends/metal/metal_types.h
    )
endif()

if (LABFONT_ENABLE_WGPU)
    set(WGPU_BACKEND_SRC
        src/backends/wgpu/wgpu_types.h
        src/backends/wgpu/wgpu_backend.h
        src/backends/wgpu/wgpu_backend.cpp
        src/backends/wgpu/wgpu_command_buffer.h
        src/backends/wgpu/wgpu_command_buffer.cpp
        src/backends/wgpu/wgpu_texture.cpp
        src/backends/wgpu/wgpu_device.cpp
        src/backends/wgpu/wgpu_render_target.cpp
    )
    
    target_compile_definitions(labfont
        PRIVATE
            LABFONT_WGPU_ENABLED=1
    )
endif()


if(LABFONT_ENABLE_VULKAN AND NOT EMSCRIPTEN)
    set(VULKAN_BACKEND_SRC
        src/backends/vulkan/vulkan_types.h
        src/backends/vulkan/vulkan_backend.h
        src/backends/vulkan/vulkan_backend.cpp
        src/backends/vulkan/vulkan_command_buffer.h
        src/backends/vulkan/vulkan_command_buffer.cpp
        src/backends/vulkan/vulkan_shader_compiler.h
        src/backends/vulkan/vulkan_shader_compiler.cpp
        src/backends/vulkan/vulkan_device.h
        src/backends/vulkan/vulkan_device.cpp
    )
    
    # Add Vulkan include directories and link libraries
    target_include_directories(labfont
        PRIVATE
            ${Vulkan_INCLUDE_DIRS}
    )
    
    target_link_libraries(labfont
        PRIVATE
            Vulkan::Vulkan
    )
    
    if(APPLE)
        target_link_libraries(labfont
            PRIVATE
                ${MOLTENVK_LIB}
        )
    endif()
    
    # Enable Vulkan backend
    target_compile_definitions(labfont
        PRIVATE
            LABFONT_VULKAN_ENABLED=1
    )
    
    # Compile SPIR-V shaders
    add_custom_command(
        OUTPUT
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/vulkan/shaders/compiled/primitives.spv
        COMMAND
            ${Vulkan_GLSLC_EXECUTABLE} -o primitives.spv primitives.vert
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/vulkan/shaders/primitives.vert
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/vulkan/shaders
        COMMENT
            "Compiling Vulkan shaders"
        VERBATIM
    )
    
    # Add shader compilation to build dependencies
    add_custom_target(vulkan_shaders
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/backends/vulkan/shaders/compiled/primitives.spv
    )
    add_dependencies(labfont vulkan_shaders)
endif()


# Source files will be added here as they are created
target_sources(labfont
    PRIVATE
        ${CORE_INCLUDES}
        ${CORE_SRC}
        ${CPU_BACKEND_SRC}
        ${METAL_BACKEND_SRC}
        ${WGPU_BACKEND_SRC}
        ${VULKAN_BACKEND_SRC}
)

if(LABFONT_ENABLE_METAL AND NOT EMSCRIPTEN)    
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

if(LABFONT_ENABLE_DX11 AND NOT EMSCRIPTEN)
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

# Installation (skip for Emscripten builds)
if(NOT EMSCRIPTEN)
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
endif()
