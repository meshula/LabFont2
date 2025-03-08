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

# on mac, prevent a terminal window from popping open by setting MACOSX_BUNDLE
add_executable(basic_drawing MACOSX_BUNDLE "basic/drawing_main.cpp")
target_link_libraries(basic_drawing
    PRIVATE
        labfont
        $<$<BOOL:${glfw3_FOUND}>:glfw>
        $<$<BOOL:${GLFW_FOUND}>:${GLFW_LIBRARIES}>
)

# set C++20 on basic_drawing
set_target_properties(basic_drawing PROPERTIES CXX_STANDARD 20)

# Include directories for LabFont headers
target_include_directories(basic_drawing
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

if(EMSCRIPTEN)
  set_target_properties(basic_drawing PROPERTIES SUFFIX ".html")
  target_link_options(basic_drawing PRIVATE "-sUSE_GLFW=3")
endif()

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
