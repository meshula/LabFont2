#!/usr/bin/env python3
"""
LabFont2 Configuration Script

This script helps configure the build environment for LabFont2 by:
1. Detecting required dependencies (Vulkan SDK, Emscripten, GLFW)
2. Generating appropriate build scripts
3. Providing guidance on missing dependencies
"""

import os
import sys
import platform
import subprocess
import shutil
from pathlib import Path

# Try to import tkinter, but don't fail if it's not available
try:
    import tkinter as tk
    from tkinter import filedialog, messagebox
    HAS_TKINTER = True
except ImportError:
    HAS_TKINTER = False
    print("Note: tkinter is not available. GUI dialogs will be replaced with command-line prompts.")

def is_command_available(command):
    """Check if a command is available on the system."""
    return shutil.which(command) is not None

def find_vulkan_sdk():
    """
    Try to find the Vulkan SDK installation.
    Returns the path to the SDK or None if not found.
    """
    # Check environment variable first
    sdk_path = os.environ.get('VULKAN_SDK')
    if sdk_path and os.path.isdir(sdk_path):
        return sdk_path
    
    # Common installation paths by platform
    if platform.system() == 'Windows':
        # Check common Windows installation paths
        program_files = os.environ.get('ProgramFiles', 'C:\\Program Files')
        paths_to_check = [
            os.path.join(program_files, 'VulkanSDK'),
            os.path.join(program_files, 'LunarG', 'VulkanSDK')
        ]
        
        # Look for the most recent version
        for base_path in paths_to_check:
            if os.path.isdir(base_path):
                # Find the highest version number
                versions = [d for d in os.listdir(base_path) if os.path.isdir(os.path.join(base_path, d))]
                if versions:
                    latest = sorted(versions)[-1]
                    return os.path.join(base_path, latest)
    
    elif platform.system() == 'Darwin':  # macOS
        # Check common macOS installation paths
        home = os.path.expanduser('~')
        paths_to_check = [
            os.path.join(home, 'VulkanSDK'),
            os.path.join(home, 'bin', 'VulkanSDK'),
            '/usr/local/VulkanSDK'
        ]
        
        # Look for the most recent version
        for base_path in paths_to_check:
            if os.path.isdir(base_path):
                # Find the highest version number
                versions = [d for d in os.listdir(base_path) if os.path.isdir(os.path.join(base_path, d))]
                if versions:
                    latest = sorted(versions)[-1]
                    sdk_path = os.path.join(base_path, latest, 'macOS')
                    if os.path.isdir(sdk_path):
                        return sdk_path
    
    elif platform.system() == 'Linux':
        # Check common Linux installation paths
        home = os.path.expanduser('~')
        paths_to_check = [
            os.path.join(home, 'VulkanSDK'),
            '/usr/local/VulkanSDK'
        ]
        
        # Look for the most recent version
        for base_path in paths_to_check:
            if os.path.isdir(base_path):
                # Find the highest version number
                versions = [d for d in os.listdir(base_path) if os.path.isdir(os.path.join(base_path, d))]
                if versions:
                    latest = sorted(versions)[-1]
                    sdk_path = os.path.join(base_path, latest, 'x86_64')
                    if os.path.isdir(sdk_path):
                        return sdk_path
    
    return None

def find_emscripten():
    """
    Try to find the Emscripten SDK installation.
    Returns the path to the emsdk_env script or None if not found.
    """
    # Check if emcc is in PATH
    if is_command_available('emcc'):
        return True
    
    # Check common installation paths
    home = os.path.expanduser('~')
    paths_to_check = [
        os.path.join(home, 'emsdk', 'emsdk_env.sh'),
        os.path.join(home, 'emsdk', 'emsdk_env.bat'),
        os.path.join(home, 'Documents', 'emsdk', 'emsdk_env.sh'),
        os.path.join(home, 'Documents', 'emsdk', 'emsdk_env.bat'),
    ]
    
    for path in paths_to_check:
        if os.path.isfile(path):
            return path
    
    return None

def find_glfw():
    """
    Try to find GLFW installation.
    Returns True if found, False otherwise.
    """
    # Check if pkg-config is available
    if is_command_available('pkg-config'):
        try:
            result = subprocess.run(['pkg-config', '--exists', 'glfw3'], check=False)
            if result.returncode == 0:
                return True
        except:
            pass
    
    # Check common installation paths
    if platform.system() == 'Windows':
        program_files = os.environ.get('ProgramFiles', 'C:\\Program Files')
        paths_to_check = [
            os.path.join(program_files, 'GLFW'),
            os.path.join(program_files, 'glfw'),
        ]
    elif platform.system() == 'Darwin':  # macOS
        paths_to_check = [
            '/usr/local/include/GLFW',
            '/usr/local/include/glfw',
            '/opt/homebrew/include/GLFW',
            '/opt/homebrew/include/glfw',
        ]
    else:  # Linux
        paths_to_check = [
            '/usr/include/GLFW',
            '/usr/include/glfw',
            '/usr/local/include/GLFW',
            '/usr/local/include/glfw',
        ]
    
    for path in paths_to_check:
        if os.path.exists(path):
            return True
    
    return False

def create_build_scripts(vulkan_sdk_path, emscripten_path, has_glfw):
    """Create build scripts based on detected dependencies."""
    script_ext = '.bat' if platform.system() == 'Windows' else '.sh'
    
    # Create build_core script (always works)
    with open(f'build_core{script_ext}', 'w') as f:
        if platform.system() == 'Windows':
            f.write('@echo off\n')
            f.write('mkdir build_core 2>nul\n')
            f.write('cd build_core\n')
            f.write('cmake .. -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF -DLABFONT_BUILD_EXAMPLES=OFF\n')
            f.write('cmake --build .\n')
            f.write('cd ..\n')
        else:
            f.write('#!/bin/bash\n')
            f.write('mkdir -p build_core && cd build_core\n')
            f.write('cmake .. -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF -DLABFONT_BUILD_EXAMPLES=OFF\n')
            f.write('make\n')
            f.write('cd ..\n')
    
    # Make the script executable on Unix-like systems
    if platform.system() != 'Windows':
        os.chmod(f'build_core{script_ext}', 0o755)
    
    # Create Vulkan build script if SDK was found
    if vulkan_sdk_path:
        with open(f'build_vk{script_ext}', 'w') as f:
            if platform.system() == 'Windows':
                f.write('@echo off\n')
                f.write('mkdir build_vk 2>nul\n')
                f.write('cd build_vk\n')
                f.write(f'set VULKAN_SDK={vulkan_sdk_path}\n')
                f.write('cmake .. -DLABFONT_ENABLE_VULKAN=ON -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                f.write('cmake --build .\n')
                f.write('cd ..\n')
            else:
                f.write('#!/bin/bash\n')
                f.write('mkdir -p build_vk && cd build_vk\n')
                f.write(f'VULKAN_SDK={vulkan_sdk_path} cmake .. -DLABFONT_ENABLE_VULKAN=ON -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                f.write('make\n')
                f.write('cd ..\n')
        
        # Make the script executable on Unix-like systems
        if platform.system() != 'Windows':
            os.chmod(f'build_vk{script_ext}', 0o755)
    
    # Create WebAssembly build script
    with open(f'build_wasm{script_ext}', 'w') as f:
        if emscripten_path:
            # If Emscripten was found, create a normal build script
            if platform.system() == 'Windows':
                f.write('@echo off\n')
                if emscripten_path is not True:  # If we found a specific path
                    f.write(f'call {emscripten_path}\n')
                f.write('mkdir build_wasm 2>nul\n')
                f.write('cd build_wasm\n')
                f.write('emcmake cmake .. -DLABFONT_ENABLE_WGPU=ON -DCMAKE_BUILD_TYPE=Debug\n')
                f.write('emmake cmake --build .\n')
                f.write('echo "Build complete. Run \'npx http-server build_wasm\' to start the server"\n')
                f.write('echo "Then open http://localhost:8080/labfont_wgpu_tests.html in Chrome"\n')
            else:
                f.write('#!/bin/bash\n')
                f.write('set -e\n\n')
                f.write('# Setup Emscripten environment\n')
                if emscripten_path is not True:  # If we found a specific path
                    f.write(f'source {emscripten_path}\n\n')
                f.write('# Create build directory\n')
                f.write('BUILD_DIR="build_wasm"\n')
                f.write('mkdir -p $BUILD_DIR\n')
                f.write('cd $BUILD_DIR\n\n')
                f.write('# Configure with CMake\n')
                f.write('emcmake cmake .. -DLABFONT_ENABLE_WGPU=ON -DCMAKE_BUILD_TYPE=Debug\n\n')
                f.write('# Build\n')
                f.write('emmake make -j$(nproc 2>/dev/null || echo 4)  # Use 4 cores if nproc not available\n\n')
                f.write('echo "Build complete. Run \'npx http-server build_wasm\' to start the server"\n')
                f.write('echo "Then open http://localhost:8080/labfont_wgpu_tests.html in Chrome"\n')
        else:
            # If Emscripten was not found, create a script that informs the user
            if platform.system() == 'Windows':
                f.write('@echo off\n')
                f.write('echo Emscripten SDK not found. WebAssembly builds require Emscripten.\n')
                f.write('echo To install Emscripten, follow these steps:\n')
                f.write('echo 1. Choose a location for Emscripten SDK (e.g., your home directory)\n')
                f.write('echo 2. Open a terminal and navigate to that location\n')
                f.write('echo 3. git clone https://github.com/emscripten-core/emsdk.git\n')
                f.write('echo 4. cd emsdk\n')
                f.write('echo 5. emsdk install latest\n')
                f.write('echo 6. emsdk activate latest\n')
                f.write('echo 7. Add the following to your shell profile (.bashrc, .zshrc, etc.):\n')
                f.write('echo    source /path/to/emsdk/emsdk_env.sh\n')
                f.write('echo 8. Run configure.py again after installation\n')
            else:
                f.write('#!/bin/bash\n')
                f.write('echo "Emscripten SDK not found. WebAssembly builds require Emscripten."\n')
                f.write('echo "To install Emscripten, follow these steps:"\n')
                f.write('echo "1. Choose a location for Emscripten SDK (e.g., your home directory)"\n')
                f.write('echo "2. Open a terminal and navigate to that location"\n')
                f.write('echo "3. git clone https://github.com/emscripten-core/emsdk.git"\n')
                f.write('echo "4. cd emsdk"\n')
                f.write('echo "5. ./emsdk install latest"\n')
                f.write('echo "6. ./emsdk activate latest"\n')
                f.write('echo "7. Add the following to your shell profile (.bashrc, .zshrc, etc.):"\n')
                f.write('echo "   source /path/to/emsdk/emsdk_env.sh"\n')
                f.write('echo "8. Run configure.py again after installation"\n')
        
    # Make the script executable on Unix-like systems
    if platform.system() != 'Windows':
        os.chmod(f'build_wasm{script_ext}', 0o755)
    
    # Create Metal build script if on macOS
    if platform.system() == 'Darwin':
        with open('build_mtl.sh', 'w') as f:
            f.write('#!/bin/bash\n')
            f.write('mkdir -p build_mtl && cd build_mtl\n')
            f.write('cmake .. -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
            f.write('make\n')
            f.write('cd ..\n')
        
        # Make the script executable
        os.chmod('build_mtl.sh', 0o755)

def update_readme(vulkan_sdk_path, emscripten_path, has_glfw):
    """Update the README.md with dependency information."""
    # Read the current README
    readme_path = 'README.md'
    if not os.path.exists(readme_path):
        # Create a basic README if it doesn't exist
        with open(readme_path, 'w') as f:
            f.write('# LabFont2\n\n')
            f.write('Modern text rendering and immediate mode drawing library\n\n')
    
    with open(readme_path, 'r') as f:
        readme_content = f.read()
    
    # Check if there's already a Dependencies section
    if '## Dependencies' in readme_content:
        # We could update the existing section, but for simplicity, let's just append our notes
        dependencies_section = '\n\n## Build Dependencies Notes\n\n'
    else:
        dependencies_section = '\n\n## Dependencies\n\n'
    
    # Add information about each dependency
    dependencies_section += 'The following dependencies are required to build different components of LabFont2:\n\n'
    
    # Vulkan SDK
    if vulkan_sdk_path:
        dependencies_section += f'- **Vulkan SDK**: Found at `{vulkan_sdk_path}`\n'
    else:
        dependencies_section += '- **Vulkan SDK**: Not found. Required for the Vulkan backend.\n'
        dependencies_section += '  - Download from [LunarG](https://www.lunarg.com/vulkan-sdk/)\n'
        dependencies_section += '  - Run the configure.py script again after installation\n'
    
    # Emscripten
    if emscripten_path:
        if emscripten_path is True:
            dependencies_section += '- **Emscripten**: Found in PATH\n'
        else:
            dependencies_section += f'- **Emscripten**: Found at `{emscripten_path}`\n'
    else:
        dependencies_section += '- **Emscripten**: Not found. Required for WebAssembly builds.\n'
        dependencies_section += '  - Follow the installation instructions at [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)\n'
        dependencies_section += '  - Run the configure.py script again after installation\n'
    
    # GLFW
    if has_glfw:
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
    script_ext = '.bat' if platform.system() == 'Windows' else '.sh'
    dependencies_section += f'- `build_core{script_ext}`: Builds the core library without any backends\n'
    
    # Vulkan backend
    if vulkan_sdk_path:
        dependencies_section += f'- `build_vk{script_ext}`: Builds with the Vulkan backend\n'
    
    # Metal backend
    if platform.system() == 'Darwin':
        dependencies_section += '- `build_mtl.sh`: Builds with the Metal backend\n'
    
    # WebAssembly
    if emscripten_path:
        dependencies_section += f'- `build_wasm{script_ext}`: Builds with WebAssembly and WebGPU\n'
    
    # Append to README
    with open(readme_path, 'a') as f:
        f.write(dependencies_section)

def update_cmake_for_glfw():
    """Update the CMakeLists.txt to better handle GLFW."""
    cmake_path = 'examples/CMakeLists.txt'
    if not os.path.exists(cmake_path):
        return
    
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

def main():
    """Main function to configure the build environment."""
    print("LabFont2 Configuration Tool")
    print("===========================")
    
    # Check for Vulkan SDK
    print("\nChecking for Vulkan SDK...")
    vulkan_sdk_path = find_vulkan_sdk()
    if vulkan_sdk_path:
        print(f"Found Vulkan SDK at: {vulkan_sdk_path}")
    else:
        print("Vulkan SDK not found automatically.")
        
        # Ask user if they want to specify the path
        if HAS_TKINTER:
            root = tk.Tk()
            root.withdraw()  # Hide the main window
            
            if messagebox.askyesno("Vulkan SDK", "Vulkan SDK not found. Would you like to specify its location?"):
                vulkan_dir = filedialog.askdirectory(title="Select Vulkan SDK Directory")
                if vulkan_dir and os.path.isdir(vulkan_dir):
                    vulkan_sdk_path = vulkan_dir
                    print(f"Using user-specified Vulkan SDK at: {vulkan_sdk_path}")
                else:
                    print("No valid Vulkan SDK directory selected.")
            else:
                print("Skipping Vulkan SDK configuration.")
        else:
            # Command-line fallback
            response = input("Vulkan SDK not found. Would you like to specify its location? (y/n): ")
            if response.lower() in ['y', 'yes']:
                vulkan_dir = input("Enter the path to the Vulkan SDK directory: ")
                if vulkan_dir and os.path.isdir(vulkan_dir):
                    vulkan_sdk_path = vulkan_dir
                    print(f"Using user-specified Vulkan SDK at: {vulkan_sdk_path}")
                else:
                    print("No valid Vulkan SDK directory provided.")
            else:
                print("Skipping Vulkan SDK configuration.")
    
    # Check for Emscripten
    print("\nChecking for Emscripten...")
    emscripten_path = find_emscripten()
    if emscripten_path:
        if emscripten_path is True:
            print("Found Emscripten in PATH")
        else:
            print(f"Found Emscripten at: {emscripten_path}")
    else:
        print("Emscripten not found.")
        print("To install Emscripten, follow the instructions at: https://emscripten.org/docs/getting_started/downloads.html")
    
    # Check for GLFW
    print("\nChecking for GLFW...")
    has_glfw = find_glfw()
    if has_glfw:
        print("Found GLFW")
    else:
        print("GLFW not found. Examples requiring GLFW will be disabled.")
        print("To install GLFW:")
        if platform.system() == 'Windows':
            print("  - Download from https://www.glfw.org/download.html or use vcpkg")
        elif platform.system() == 'Darwin':
            print("  - Install with Homebrew: brew install glfw")
        else:  # Linux
            print("  - Install with your package manager, e.g., apt install libglfw3-dev")
    
    # Create build scripts
    print("\nGenerating build scripts...")
    create_build_scripts(vulkan_sdk_path, emscripten_path, has_glfw)
    print("Build scripts created successfully.")
    
    # Update README
    print("\nUpdating README with dependency information...")
    update_readme(vulkan_sdk_path, emscripten_path, has_glfw)
    print("README updated successfully.")
    
    # Update CMake for GLFW
    print("\nUpdating CMake configuration for GLFW...")
    update_cmake_for_glfw()
    print("CMake configuration updated successfully.")
    
    print("\nConfiguration complete!")
    print("You can now use the generated build scripts to build LabFont2.")

if __name__ == "__main__":
    main()
