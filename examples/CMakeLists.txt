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

add_executable(basic_drawing "basic/drawing_main.cpp")
target_link_libraries(basic_drawing
    PRIVATE
        labfont
)

# set C++20 on basic_drawing
set_target_properties(basic_drawing PROPERTIES CXX_STANDARD 20)

if(EMSCRIPTEN)
  set_target_properties(basic_drawing PROPERTIES SUFFIX ".html")
  target_link_options(basic_drawing PRIVATE "-sUSE_WEBGPU=1" "-sUSE_GLFW=3")
else()
#  set(DAWN_FETCH_DEPENDENCIES ON)
#  add_subdirectory("dawn" EXCLUDE_FROM_ALL)
#  target_link_libraries(app PRIVATE dawn::webgpu_dawn glfw webgpu_glfw)
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
