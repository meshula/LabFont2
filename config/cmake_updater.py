"""
CMake Updater Module

This module contains the CMakeUpdater class for updating CMake files
to better handle dependencies like GLFW.
"""

import os


class CMakeUpdater:
    """
    Class for updating CMake files to better handle dependencies.
    """

    def __init__(self):
        """Initialize the CMakeUpdater."""
        pass

    def update_cmake_for_glfw(self):
        """
        Update the CMakeLists.txt to better handle GLFW.
        
        Returns:
            bool: True if the CMakeLists.txt was updated, False otherwise.
        """
        cmake_path = 'examples/CMakeLists.txt'
        if not os.path.exists(cmake_path):
            return False
        
        with open(cmake_path, 'r') as f:
            content = f.read()
        
        # Check if we need to add GLFW handling
        if 'find_package(glfw3' not in content:
            # Add GLFW handling before the add_executable line
            updated_content = content.replace(
                'add_executable(basic_drawing "basic/drawing_main.cpp")',
                '# Find GLFW\nfind_package(glfw3 QUIET)\nif(NOT glfw3_FOUND)\n  find_package(PkgConfig QUIET)\n  if(PkgConfig_FOUND)\n    pkg_check_modules(GLFW QUIET glfw3)\n  endif()\nendif()\n\n# Check if GLFW was found\nif(NOT glfw3_FOUND AND NOT GLFW_FOUND)\n  message(WARNING "GLFW not found. Examples requiring GLFW will be disabled.")\n  return()\nendif()\n\nadd_executable(basic_drawing "basic/drawing_main.cpp")'
            )
            
            # Update the target_link_libraries line to include GLFW
            updated_content = updated_content.replace(
                'target_link_libraries(basic_drawing\n    PRIVATE\n        labfont\n)',
                'target_link_libraries(basic_drawing\n    PRIVATE\n        labfont\n        $<$<BOOL:${glfw3_FOUND}>:glfw>\n        $<$<BOOL:${GLFW_FOUND}>:${GLFW_LIBRARIES}>\n)'
            )
            
            with open(cmake_path, 'w') as f:
                f.write(updated_content)
            
            return True
        
        return False
