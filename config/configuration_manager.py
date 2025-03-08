"""
Configuration Manager Module

This module contains the ConfigurationManager class that orchestrates the
configuration process by using the other classes.
"""

import os
import platform

from .dependency_detector import DependencyDetector
from .script_generator import ScriptGenerator
from .readme_updater import ReadmeUpdater
from .cmake_updater import CMakeUpdater


class ConfigurationManager:
    """
    Class that orchestrates the configuration process.
    """

    def __init__(self):
        """Initialize the ConfigurationManager."""
        self.dependency_detector = DependencyDetector()
        self.vulkan_sdk_path = None
        self.emscripten_path = None
        self.has_glfw = False
        self.has_tkinter = self._check_tkinter()

    @staticmethod
    def _check_tkinter():
        """
        Check if tkinter is available.
        
        Returns:
            bool: True if tkinter is available, False otherwise.
        """
        try:
            import tkinter
            return True
        except ImportError:
            print("Note: tkinter is not available. GUI dialogs will be replaced with command-line prompts.")
            return False

    def configure(self):
        """
        Run the configuration process.
        
        Returns:
            bool: True if the configuration was successful, False otherwise.
        """
        print("LabFont2 Configuration Tool")
        print("===========================")
        
        # Detect dependencies
        self._detect_dependencies()
        
        # Generate build scripts
        self._generate_build_scripts()
        
        # Update README
        self._update_readme()
        
        # Update CMake files
        self._update_cmake_files()
        
        print("\nConfiguration complete!")
        print("You can now use the generated build scripts to build LabFont2.")
        
        return True

    def _detect_dependencies(self):
        """Detect dependencies and prompt for missing ones."""
        # Check for Vulkan SDK
        print("\nChecking for Vulkan SDK...")
        self.vulkan_sdk_path = self.dependency_detector.find_vulkan_sdk()
        if self.vulkan_sdk_path:
            print(f"Found Vulkan SDK at: {self.vulkan_sdk_path}")
        else:
            print("Vulkan SDK not found automatically.")
            
            # Ask user if they want to specify the path
            if self.has_tkinter:
                try:
                    import tkinter as tk
                    from tkinter import filedialog, messagebox
                    
                    root = tk.Tk()
                    root.withdraw()  # Hide the main window
                    
                    if messagebox.askyesno("Vulkan SDK", "Vulkan SDK not found. Would you like to specify its location?"):
                        vulkan_dir = filedialog.askdirectory(title="Select Vulkan SDK Directory")
                        if vulkan_dir and os.path.isdir(vulkan_dir):
                            self.vulkan_sdk_path = vulkan_dir
                            print(f"Using user-specified Vulkan SDK at: {self.vulkan_sdk_path}")
                        else:
                            print("No valid Vulkan SDK directory selected.")
                    else:
                        print("Skipping Vulkan SDK configuration.")
                except Exception as e:
                    print(f"Error using tkinter: {e}")
                    print("Falling back to command-line prompts.")
                    self.has_tkinter = False
                    # Fall back to command-line prompts
                    response = input("Vulkan SDK not found. Would you like to specify its location? (y/n): ")
                    if response.lower() in ['y', 'yes']:
                        vulkan_dir = input("Enter the path to the Vulkan SDK directory: ")
                        if vulkan_dir and os.path.isdir(vulkan_dir):
                            self.vulkan_sdk_path = vulkan_dir
                            print(f"Using user-specified Vulkan SDK at: {self.vulkan_sdk_path}")
                        else:
                            print("No valid Vulkan SDK directory provided.")
                    else:
                        print("Skipping Vulkan SDK configuration.")
            else:
                # Command-line fallback
                response = input("Vulkan SDK not found. Would you like to specify its location? (y/n): ")
                if response.lower() in ['y', 'yes']:
                    vulkan_dir = input("Enter the path to the Vulkan SDK directory: ")
                    if vulkan_dir and os.path.isdir(vulkan_dir):
                        self.vulkan_sdk_path = vulkan_dir
                        print(f"Using user-specified Vulkan SDK at: {self.vulkan_sdk_path}")
                    else:
                        print("No valid Vulkan SDK directory provided.")
                else:
                    print("Skipping Vulkan SDK configuration.")
        
        # Check for Emscripten
        print("\nChecking for Emscripten...")
        self.emscripten_path = self.dependency_detector.find_emscripten()
        if self.emscripten_path:
            if self.emscripten_path is True:
                print("Found Emscripten in PATH")
            else:
                print(f"Found Emscripten at: {self.emscripten_path}")
        else:
            print("Emscripten not found.")
            print("To install Emscripten, follow the instructions at: https://emscripten.org/docs/getting_started/downloads.html")
        
        # Check for GLFW
        print("\nChecking for GLFW...")
        self.has_glfw = self.dependency_detector.find_glfw()
        if self.has_glfw:
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

    def _generate_build_scripts(self):
        """Generate build scripts based on detected dependencies."""
        print("\nGenerating build scripts...")
        script_generator = ScriptGenerator(
            vulkan_sdk_path=self.vulkan_sdk_path,
            emscripten_path=self.emscripten_path,
            has_glfw=self.has_glfw
        )
        script_generator.generate_all_scripts()
        print("Build scripts created successfully.")

    def _update_readme(self):
        """Update README with dependency information."""
        print("\nUpdating README with dependency information...")
        readme_updater = ReadmeUpdater(
            vulkan_sdk_path=self.vulkan_sdk_path,
            emscripten_path=self.emscripten_path,
            has_glfw=self.has_glfw
        )
        readme_updater.update_readme()
        print("README updated successfully.")

    def _update_cmake_files(self):
        """Update CMake files to better handle dependencies."""
        print("\nUpdating CMake configuration for GLFW...")
        cmake_updater = CMakeUpdater()
        cmake_updater.update_cmake_for_glfw()
        print("CMake configuration updated successfully.")
