"""
Script Generator Module

This module contains the ScriptGenerator class for generating build scripts
for different backends (CPU, Vulkan, Metal, WebGPU).
"""

import os
import platform


class ScriptGenerator:
    """
    Class for generating build scripts for different backends.
    """

    def __init__(self, vulkan_sdk_path=None, emscripten_path=None, has_glfw=False, use_xcode=False, build_dir="build"):
        """
        Initialize the ScriptGenerator.
        
        Args:
            vulkan_sdk_path (str, optional): Path to the Vulkan SDK.
            emscripten_path (str or bool, optional): Path to Emscripten or True if in PATH.
            has_glfw (bool, optional): Whether GLFW is available.
            use_xcode (bool, optional): Whether to use Xcode generator instead of make. Defaults to False.
            build_dir (str, optional): Directory to place the build scripts. Defaults to "build".
        """
        self.vulkan_sdk_path = vulkan_sdk_path
        self.emscripten_path = emscripten_path
        self.has_glfw = has_glfw
        self.use_xcode = use_xcode
        self.build_dir = build_dir
        self.script_ext = '.bat' if platform.system() == 'Windows' else '.sh'
        self.source_dir = os.getcwd()

    def generate_all_scripts(self):
        """Generate all build scripts based on detected dependencies."""
        self.generate_core_script()
        
        if self.has_glfw:
            self.generate_examples_cpu_script()
            
            if self.vulkan_sdk_path:
                self.generate_examples_vulkan_script()
            
            if platform.system() == 'Darwin':
                self.generate_examples_metal_script()
            
            if self.emscripten_path:
                self.generate_examples_wgpu_script()
        
        if self.vulkan_sdk_path:
            self.generate_vulkan_script()
        
        self.generate_wasm_script()
        
        if platform.system() == 'Darwin':
            self.generate_metal_script()

    def generate_core_script(self):
        """Generate the core build script."""
        os.makedirs(self.build_dir, exist_ok=True)
        script_path = os.path.join(self.build_dir, f'build_core{self.script_ext}')
        with open(script_path, 'w') as f:
            if platform.system() == 'Windows':
                f.write('@echo off\n')
                f.write('mkdir build_core 2>nul\n')
                f.write('cd build_core\n')
                f.write(f'cmake {self.source_dir} -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF -DLABFONT_BUILD_EXAMPLES=OFF\n')
                f.write('cmake --build .\n')
                f.write('cd ..\n')
            else:
                f.write('#!/bin/bash\n')
                f.write('mkdir -p build_core && cd build_core\n')
                
                # Use Xcode generator if specified and on macOS
                if self.use_xcode and platform.system() == 'Darwin':
                    f.write(f'cmake {self.source_dir} -G Xcode -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF -DLABFONT_BUILD_EXAMPLES=OFF\n')
                    f.write('cmake --build . --config Release\n')
                else:
                    f.write(f'cmake {self.source_dir} -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF -DLABFONT_BUILD_EXAMPLES=OFF\n')
                    f.write('make\n')
                
                f.write('cd ..\n')
        
        # Make the script executable on Unix-like systems
        if platform.system() != 'Windows':
            os.chmod(script_path, 0o755)

    def generate_examples_cpu_script(self):
        """Generate the examples cpubuild script."""
        os.makedirs(self.build_dir, exist_ok=True)
        script_path = os.path.join(self.build_dir, f'build_examples_cpu{self.script_ext}')
        with open(script_path, 'w') as f:
            if platform.system() == 'Windows':
                f.write('@echo off\n')
                f.write('mkdir build_examples_cpu 2>nul\n')
                f.write('cd build_examples_cpu\n')
                f.write(f'cmake {self.source_dir} -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                f.write('cmake --build .\n')
                f.write('cd ..\n')
            else:
                f.write('#!/bin/bash\n')
                f.write('mkdir -p build_examples_cpu && cd build_examples_cpu\n')
                
                # Use Xcode generator if specified and on macOS
                if self.use_xcode and platform.system() == 'Darwin':
                    f.write(f'cmake {self.source_dir} -G Xcode -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                    f.write('cmake --build . --config Release\n')
                else:
                    f.write(f'cmake {self.source_dir} -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                    f.write('make\n')
                
                f.write('cd ..\n')
        
        # Make the script executable on Unix-like systems
        if platform.system() != 'Windows':
            os.chmod(script_path, 0o755)

    def generate_examples_vulkan_script(self):
        """Generate the Vulkan examples build script."""
        os.makedirs(self.build_dir, exist_ok=True)
        script_path = os.path.join(self.build_dir, f'build_examples_vulkan{self.script_ext}')
        with open(script_path, 'w') as f:
            if platform.system() == 'Windows':
                f.write('@echo off\n')
                f.write('mkdir build_examples_vulkan 2>nul\n')
                f.write('cd build_examples_vulkan\n')
                f.write(f'set VULKAN_SDK={self.vulkan_sdk_path}\n')
                f.write(f'cmake {self.source_dir} -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_VULKAN=ON -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                f.write('cmake --build .\n')
                f.write('cd ..\n')
            else:
                f.write('#!/bin/bash\n')
                f.write('mkdir -p build_examples_vulkan && cd build_examples_vulkan\n')
                
                # Use Xcode generator if specified and on macOS
                if self.use_xcode and platform.system() == 'Darwin':
                    f.write(f'VULKAN_SDK={self.vulkan_sdk_path} cmake .. -G Xcode -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_VULKAN=ON -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                    f.write('cmake --build . --config Release\n')
                else:
                    f.write(f'VULKAN_SDK={self.vulkan_sdk_path} cmake .. -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_VULKAN=ON -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                    f.write('make\n')
                
                f.write('cd ..\n')
        
        # Make the script executable on Unix-like systems
        if platform.system() != 'Windows':
            os.chmod(script_path, 0o755)

    def generate_examples_metal_script(self):
        """Generate the Metal examples build script."""
        os.makedirs(self.build_dir, exist_ok=True)
        script_path = os.path.join(self.build_dir, f'build_examples_metal{self.script_ext}')
        with open(script_path, 'w') as f:
            f.write('#!/bin/bash\n')
            f.write('mkdir -p build_examples_metal && cd build_examples_metal\n')
            
            # Use Xcode generator if specified
            if self.use_xcode:
                f.write(f'cmake {self.source_dir} -G Xcode -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                f.write('cmake --build . --config Release\n')
            else:
                f.write(f'cmake {self.source_dir} -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                f.write('make\n')
            
            f.write('cd ..\n')
        
        # Make the script executable
        os.chmod(script_path, 0o755)

    def generate_examples_wgpu_script(self):
        """Generate the WebGPU examples build script."""
        os.makedirs(self.build_dir, exist_ok=True)
        script_path = os.path.join(self.build_dir, f'build_examples_wgpu{self.script_ext}')
        with open(script_path, 'w') as f:
            if platform.system() == 'Windows':
                f.write('@echo off\n')
                if self.emscripten_path is not True:  # If we found a specific path
                    f.write(f'call {self.emscripten_path}\n')
                f.write('mkdir build_examples_wgpu 2>nul\n')
                f.write('cd build_examples_wgpu\n')
                f.write('emcmake cmake .. -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_WGPU=ON -DCMAKE_BUILD_TYPE=Debug\n')
                f.write('emmake cmake --build .\n')
                f.write('echo "Build complete. Run \'npx http-server build_examples_wgpu\' to start the server"\n')
                f.write('echo "Then open http://localhost:8080/examples/basic_drawing.html in Chrome"\n')
            else:
                f.write('#!/bin/bash\n')
                f.write('set -e\n\n')
                f.write('# Setup Emscripten environment\n')
                if self.emscripten_path is not True:  # If we found a specific path
                    f.write(f'source {self.emscripten_path}\n\n')
                f.write('# Create build directory\n')
                f.write('BUILD_DIR="build_examples_wgpu"\n')
                f.write('mkdir -p $BUILD_DIR\n')
                f.write('cd $BUILD_DIR\n\n')
                f.write('# Configure with CMake\n')
                f.write('emcmake cmake .. -DLABFONT_BUILD_EXAMPLES=ON -DLABFONT_BUILD_TESTS=OFF -DLABFONT_ENABLE_WGPU=ON -DCMAKE_BUILD_TYPE=Debug\n\n')
                f.write('# Build\n')
                f.write('emmake make -j$(nproc 2>/dev/null || echo 4)  # Use 4 cores if nproc not available\n\n')
                f.write('echo "Build complete. Run \'npx http-server build_examples_wgpu\' to start the server"\n')
                f.write('echo "Then open http://localhost:8080/examples/basic_drawing.html in Chrome"\n')
        
        # Make the script executable on Unix-like systems
        if platform.system() != 'Windows':
            os.chmod(script_path, 0o755)

    def generate_vulkan_script(self):
        """Generate the Vulkan build script."""
        os.makedirs(self.build_dir, exist_ok=True)
        script_path = os.path.join(self.build_dir, f'build_vk{self.script_ext}')
        with open(script_path, 'w') as f:
            if platform.system() == 'Windows':
                f.write('@echo off\n')
                f.write('mkdir build_vk 2>nul\n')
                f.write('cd build_vk\n')
                f.write(f'set VULKAN_SDK={self.vulkan_sdk_path}\n')
                f.write(f'cmake {self.source_dir} -DLABFONT_ENABLE_VULKAN=ON -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                f.write('cmake --build .\n')
                f.write('cd ..\n')
            else:
                f.write('#!/bin/bash\n')
                f.write('mkdir -p build_vk && cd build_vk\n')
                
                # Use Xcode generator if specified and on macOS
                if self.use_xcode and platform.system() == 'Darwin':
                    f.write(f'VULKAN_SDK={self.vulkan_sdk_path} cmake .. -G Xcode -DLABFONT_ENABLE_VULKAN=ON -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                    f.write('cmake --build . --config Release\n')
                else:
                    f.write(f'VULKAN_SDK={self.vulkan_sdk_path} cmake .. -DLABFONT_ENABLE_VULKAN=ON -DLABFONT_ENABLE_METAL=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                    f.write('make\n')
                
                f.write('cd ..\n')
        
        # Make the script executable on Unix-like systems
        if platform.system() != 'Windows':
            os.chmod(script_path, 0o755)

    def generate_wasm_script(self):
        """Generate the WebAssembly build script."""
        os.makedirs(self.build_dir, exist_ok=True)
        script_path = os.path.join(self.build_dir, f'build_wasm{self.script_ext}')
        with open(script_path, 'w') as f:
            if self.emscripten_path:
                # If Emscripten was found, create a normal build script
                if platform.system() == 'Windows':
                    f.write('@echo off\n')
                    if self.emscripten_path is not True:  # If we found a specific path
                        f.write(f'call {self.emscripten_path}\n')
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
                    if self.emscripten_path is not True:  # If we found a specific path
                        f.write(f'source {self.emscripten_path}\n\n')
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
            os.chmod(script_path, 0o755)

    def generate_metal_script(self):
        """Generate the Metal build script."""
        os.makedirs(self.build_dir, exist_ok=True)
        script_path = os.path.join(self.build_dir, f'build_metal{self.script_ext}')
        with open(script_path, 'w') as f:
            f.write('#!/bin/bash\n')
            f.write('mkdir -p build_mtl && cd build_mtl\n')
            
            # Use Xcode generator if specified
            if self.use_xcode:
                f.write(f'cmake {self.source_dir} -G Xcode -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                f.write('cmake --build . --config Release\n')
            else:
                f.write(f'cmake {self.source_dir} -DLABFONT_ENABLE_METAL=ON -DLABFONT_ENABLE_VULKAN=OFF -DLABFONT_ENABLE_WGPU=OFF\n')
                f.write('make\n')
            
            f.write('cd ..\n')
        
        # Make the script executable
        os.chmod(script_path, 0o755)
