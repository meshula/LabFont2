# Examples will be added as we implement features

# Basic examples
# add_executable(basic_text_rendering)
# target_sources(basic_text_rendering
#     PRIVATE
#         basic/text_rendering.c
# )
# target_link_libraries(basic_text_rendering
#     PRIVATE
#         labfont
# )

# Find GLFW
find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(GLFW QUIET glfw3)
  endif()
endif()

# Check if GLFW was found
if(NOT glfw3_FOUND AND NOT GLFW_FOUND)
  message(WARNING "GLFW not found. Examples requiring GLFW will be disabled.")
  return()
endif()

set(LAB_BACKEND "" CACHE STRING "The backend to use for rendering")

# validate the backend
if(NOT LAB_BACKEND STREQUAL "CPU" AND
   NOT LAB_BACKEND STREQUAL "METAL" AND
   NOT LAB_BACKEND STREQUAL "WEBGPU" AND
   NOT LAB_BACKEND STREQUAL "DX11" AND
   NOT LAB_BACKEND STREQUAL "VULKAN")
  message(FATAL_ERROR "Invalid LAB_BACKEND: ${LAB_BACKEND}, must be one of CPU, METAL, WEBGPU, DX11,VULKAN")
endif()

# Set the resolved backend
if(LAB_BACKEND STREQUAL "CPU")
  set(RESOLVED_BACKEND "LAB_BACKEND_CPU")
elseif(LAB_BACKEND STREQUAL "METAL")
  set(RESOLVED_BACKEND "LAB_BACKEND_METAL")
elseif(LAB_BACKEND STREQUAL "WEBGPU")
  set(RESOLVED_BACKEND "LAB_BACKEND_WEBGPU")
elseif(LAB_BACKEND STREQUAL "DX11")
  set(RESOLVED_BACKEND "LAB_BACKEND_DX11")
elseif(LAB_BACKEND STREQUAL "VULKAN")
  set(RESOLVED_BACKEND "LAB_BACKEND_VULKAN")
endif()


# on Apple platforms, the main extension is .mm, otherwise .cpp
if(APPLE)
  set(MAIN_EXTENSION "mm")
else()
  set(MAIN_EXTENSION "cpp")
endif()

# setting up the Apple specific build is very complex. we should create a function that
# takes the target name and the source file and sets up the target properly
# this will make it easier to add more examples in the future
# we should also create the drawing_main, texture_main, and labfont_text_rendering demos using that function.

# function: add_labfont_example
# parameters:
#   target_name: the name of the target
#   source_file: the source file for the target
#   backend: the backend to use for rendering
#   resources: the resources to copy to the build directory
function(add_labfont_example target_name source_file backend resources)
  if(APPLE)
    add_executable(${target_name} MACOSX_BUNDLE ${source_file}.${MAIN_EXTENSION})
    # Add drawing_main.cpp as a source file but prevent it from being compiled, so that it shows up in xcode
   target_sources(${target_name} PRIVATE ${source_file}.cpp)
   set_source_files_properties(${source_file}.cpp PROPERTIES HEADER_FILE_ONLY ON)
  else()
    add_executable(${target_name} ${source_file}.${MAIN_EXTENSION})
  endif()

  # Link libraries for the target
  target_link_libraries(${target_name}
      PRIVATE
          labfont
          $<$<BOOL:${glfw3_FOUND}>:glfw>
          $<$<BOOL:${GLFW_FOUND}>:${GLFW_LIBRARIES}>
  )

  if(APPLE)
    target_link_libraries(${target_name}
        PRIVATE
            "-framework CoreGraphics"
            "-framework QuartzCore"
            "-framework Cocoa"
            "-framework Foundation"
            "-framework Metal"
    )

    set(ENTITLEMENTS_PATH "${CMAKE_SOURCE_DIR}/src/Lab.entitlements") 
    set(APP_VERSION "1.0.0")
    set_target_properties(${target_name} PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
        OUTPUT_NAME "${target_name}"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "${target_name}"
        #MACOSX_BUNDLE_INFO_PLIST path/to/Info.plist
        MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VERSION}
        MACOSX_BUNDLE_LONG_VERSION_STRING "${target_name} v${APP_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${APP_VERSION}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.planetix.labfont.basicdrawing"
        MACOSX_BUNDLE_COPYRIGHT "(C) 2025 Nick Porcino"
        MACOSX_RPATH TRUE
        MACOSX_FRAMEWORK_IDENTIFIER com.planetix.labfont.basicdrawing
        XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/Libraries"
        RESOURCE "${RESOURCE_FILES}"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME FALSE
        XCODE_ATTRIBUTE_EXECUTABLE_NAME "${target_name}"
    )
  endif()

  # set C++20 on the target
  set_target_properties(${target_name} PROPERTIES CXX_STANDARD 20)

  # Include directories for LabFont headers
  target_include_directories(${target_name}
      PRIVATE
          ${CMAKE_SOURCE_DIR}/include
  )

  # Set the backend for the target
  target_compile_definitions(${target_name} PRIVATE LAB_BACKEND=${backend})

  # Copy resources folder to build directory for the target
  add_custom_command(TARGET ${target_name} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/resources
          $<TARGET_FILE_DIR:${target_name}>/resources
      COMMENT "Copying resources folder to ${target_name} build directory"
  )

  # Copy Metal shader library to test executable directory
  if(APPLE)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/src/backends/metal/shaders/compiled/primitives.metallib
            $<TARGET_FILE_DIR:${target_name}>/primitives.metallib
        COMMENT "Copying Metal shader library to ${target_name} build directory"
    )
  endif()

  if(EMSCRIPTEN)
    set_target_properties(${target_name} PROPERTIES SUFFIX ".html")
    target_link_options(${target_name} PRIVATE "-sUSE_GLFW=3")
  endif()
endfunction()

# Basic drawing example
add_labfont_example(basic_drawing "basic/drawing_main" ${RESOLVED_BACKEND} "resources")

# Texture example
add_labfont_example(texture_example "basic/texture_main" ${RESOLVED_BACKEND} "resources")

# Rich text
add_labfont_example(rich_text "rich_text/labfont_text_rendering" ${RESOLVED_BACKEND} "resources")
