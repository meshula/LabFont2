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

if(APPLE)
  # Basic drawing example
  add_executable(basic_drawing MACOSX_BUNDLE "basic/drawing_main.mm")

  # Add drawing_main.cpp as a source file but prevent it from being compiled, so that it shows up in xcode
  target_sources(basic_drawing PRIVATE basic/drawing_main.cpp)
  set_source_files_properties(basic/drawing_main.cpp PROPERTIES HEADER_FILE_ONLY ON)
  
  # Texture example
  add_executable(texture_example MACOSX_BUNDLE "basic/drawing_main.mm")
  
  # Add texture_main.cpp as a source file but prevent it from being compiled, so that it shows up in xcode
  target_sources(texture_example PRIVATE basic/texture_main.cpp)
  set_source_files_properties(basic/texture_main.cpp PROPERTIES HEADER_FILE_ONLY ON)

  target_link_libraries(basic_drawing
      PRIVATE
          "-framework CoreGraphics"
          "-framework QuartzCore"
          "-framework Cocoa"
          "-framework Metal"
  )

  # Copy Metal shader library to test executable directory
  add_custom_command(TARGET basic_drawing POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
          ${CMAKE_SOURCE_DIR}/src/backends/metal/shaders/compiled/primitives.metallib
          $<TARGET_FILE_DIR:basic_drawing>/primitives.metallib
      COMMENT "Copying Metal shader library to test executable directory"
  )

  set(ENTITLEMENTS_PATH "${CMAKE_SOURCE_DIR}/src/Lab.entitlements")
    
  set(APP_VERSION "1.0.0")
  set_target_properties(basic_drawing PROPERTIES
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
    OUTPUT_NAME "Lab Font Basic Drawing Example"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "Lab Font Basic Drawing Example"
    #MACOSX_BUNDLE_INFO_PLIST path/to/Info.plist
    MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VERSION}
    MACOSX_BUNDLE_LONG_VERSION_STRING "My nice application name v${APP_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${APP_VERSION}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.planetix.labfont.basicdrawing"
    MACOSX_BUNDLE_COPYRIGHT "(C) 2025 Nick Porcino"
    MACOSX_RPATH TRUE
    MACOSX_FRAMEWORK_IDENTIFIER com.planetix.labfont.basicdrawing
    XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@loader_path/Libraries"
    RESOURCE "${RESOURCE_FILES}"
    XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME FALSE
    XCODE_ATTRIBUTE_EXECUTABLE_NAME "Lab Font Basic Drawing Example"
  )
else()
  add_executable(basic_drawing "basic/drawing_main.cpp")
  add_executable(texture_example "basic/texture_main.cpp")
endif()
# Link libraries for basic_drawing
target_link_libraries(basic_drawing
    PRIVATE
        labfont
        $<$<BOOL:${glfw3_FOUND}>:glfw>
        $<$<BOOL:${GLFW_FOUND}>:${GLFW_LIBRARIES}>
)

# Link libraries for texture_example
target_link_libraries(texture_example
    PRIVATE
        labfont
        $<$<BOOL:${glfw3_FOUND}>:glfw>
        $<$<BOOL:${GLFW_FOUND}>:${GLFW_LIBRARIES}>
)

# set C++20 on examples
set_target_properties(basic_drawing PROPERTIES CXX_STANDARD 20)
set_target_properties(texture_example PROPERTIES CXX_STANDARD 20)

# Include directories for LabFont headers
target_include_directories(basic_drawing
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(texture_example
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

if(EMSCRIPTEN)
  set_target_properties(basic_drawing PROPERTIES SUFFIX ".html")
  target_link_options(basic_drawing PRIVATE "-sUSE_GLFW=3")
  
  set_target_properties(texture_example PROPERTIES SUFFIX ".html")
  target_link_options(texture_example PRIVATE "-sUSE_GLFW=3")
endif()

if(NOT RESOLVED_BACKEND)
  message(FATAL_ERROR "RESOLVED_BACKEND is not set")
endif()

target_compile_definitions(basic_drawing PRIVATE LAB_BACKEND=${RESOLVED_BACKEND})
target_compile_definitions(texture_example PRIVATE LAB_BACKEND=${RESOLVED_BACKEND})

# Copy resources folder to build directory for examples
add_custom_command(TARGET texture_example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:texture_example>/resources
    COMMENT "Copying resources folder to texture_example build directory"
)

# Advanced examples
# add_executable(advanced_text_styling)
# target_sources(advanced_text_styling
#     PRIVATE
#         advanced/text_styling.c
# )
# target_link_libraries(advanced_text_styling
#     PRIVATE
#         labfont
# )

# add_executable(advanced_drawing)
# target_sources(advanced_drawing
#     PRIVATE
#         advanced/complex_drawing.c
# )
# target_link_libraries(advanced_drawing
#     PRIVATE
#         labfont
# )
