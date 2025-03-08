"""
README Updater Module

This module contains the ReadmeUpdater class for updating the README.md file
with dependency information.
"""

import os
import platform


class ReadmeUpdater:
    """
    Class for updating the README.md file with dependency information.
    """

    def __init__(self, vulkan_sdk_path=None, emscripten_path=None, has_glfw=False):
        """
        Initialize the ReadmeUpdater.
        
        Args:
            vulkan_sdk_path (str, optional): Path to the Vulkan SDK.
            emscripten_path (str or bool, optional): Path to Emscripten or True if in PATH.
            has_glfw (bool, optional): Whether GLFW is available.
        """
        self.vulkan_sdk_path = vulkan_sdk_path
        self.emscripten_path = emscripten_path
        self.has_glfw = has_glfw
        self.script_ext = '.bat' if platform.system() == 'Windows' else '.sh'

    def update_readme(self):
        """
        Update the README.md with dependency information.
        
        Returns:
            bool: True if the README was updated, False otherwise.
        """
        # Read the current README
        readme_path = 'README.md'
        if not os.path.exists(readme_path):
            # Create a basic README if it doesn't exist
            with open(readme_path, 'w') as f:
                f.write('# LabFont2\n\n')
                f.write('Modern text rendering and immediate mode drawing library\n\n')
        
        with open(readme_path, 'r') as f:
            readme_content = f.read()
        
        # Create the dependencies section
        dependencies_section = '## Build Dependencies Notes\n\n'
        dependencies_section += 'The following dependencies are required to build different components of LabFont2:\n\n'
        
        # Vulkan SDK
        if self.vulkan_sdk_path:
            dependencies_section += f'- **Vulkan SDK**: Found at `{self.vulkan_sdk_path}`\n'
        else:
            dependencies_section += '- **Vulkan SDK**: Not found. Required for the Vulkan backend.\n'
            dependencies_section += '  - Download from [LunarG](https://www.lunarg.com/vulkan-sdk/)\n'
            dependencies_section += '  - Run the configure.py script again after installation\n'
        
        # Emscripten
        if self.emscripten_path:
            if self.emscripten_path is True:
                dependencies_section += '- **Emscripten**: Found in PATH\n'
            else:
                dependencies_section += f'- **Emscripten**: Found at `{self.emscripten_path}`\n'
        else:
            dependencies_section += '- **Emscripten**: Not found. Required for WebAssembly builds.\n'
            dependencies_section += '  - Follow the installation instructions at [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)\n'
            dependencies_section += '  - Run the configure.py script again after installation\n'
        
        # GLFW
        if self.has_glfw:
            dependencies_section += '- **GLFW**: Found\n'
        else:
            dependencies_section += '- **GLFW**: Not found. Required for examples.\n'
            if platform.system() == 'Windows':
                dependencies_section += '  - Download from [GLFW](https://www.glfw.org/download.html) or use vcpkg\n'
            elif platform.system() == 'Darwin':
                dependencies_section += '  - Install with Homebrew: `brew install glfw`\n'
            else:  # Linux
                dependencies_section += '  - Install with your package manager, e.g., `apt install libglfw3-dev`\n'
            dependencies_section += '  - Run the configure.py script again after installation\n'
        
        # Add build instructions
        dependencies_section += '\n## Building\n\n'
        dependencies_section += 'The configure.py script has generated build scripts for the detected dependencies:\n\n'
        
        # Core library (always available)
        dependencies_section += f'- `build_core{self.script_ext}`: Builds the core library without any backends\n'
        
        # Vulkan backend
        if self.vulkan_sdk_path:
            dependencies_section += f'- `build_vk{self.script_ext}`: Builds with the Vulkan backend\n'
        
        # Metal backend
        if platform.system() == 'Darwin':
            dependencies_section += '- `build_mtl.sh`: Builds with the Metal backend\n'
        
        # WebAssembly
        if self.emscripten_path:
            dependencies_section += f'- `build_wasm{self.script_ext}`: Builds with WebAssembly and WebGPU\n'
        
        # Example build scripts
        if self.has_glfw:
            dependencies_section += '\n### Example Build Scripts\n\n'
            dependencies_section += f'- `build_examples_cpu{self.script_ext}`: Builds examples with the CPU backend\n'
            
            if self.vulkan_sdk_path:
                dependencies_section += f'- `build_examples_vulkan{self.script_ext}`: Builds examples with the Vulkan backend\n'
            
            if platform.system() == 'Darwin':
                dependencies_section += '- `build_examples_metal.sh`: Builds examples with the Metal backend\n'
            
            if self.emscripten_path:
                dependencies_section += f'- `build_examples_wgpu{self.script_ext}`: Builds examples with the WebGPU backend\n'
        
        # Check if there's already a Build Dependencies Notes section
        build_deps_index = readme_content.find('## Build Dependencies Notes')
        if build_deps_index != -1:
            # Replace the existing section
            new_readme_content = readme_content[:build_deps_index] + dependencies_section
            with open(readme_path, 'w') as f:
                f.write(new_readme_content)
        else:
            # Append to README
            with open(readme_path, 'a') as f:
                f.write('\n\n' + dependencies_section)
        
        return True
