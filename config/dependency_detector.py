"""
Dependency Detector Module

This module contains the DependencyDetector class for finding dependencies
required by LabFont2, such as Vulkan SDK, Emscripten, and GLFW.
"""

import os
import platform
import subprocess
import shutil
from pathlib import Path


class DependencyDetector:
    """
    Class for detecting dependencies required by LabFont2.
    """

    def __init__(self):
        """Initialize the DependencyDetector."""
        self.vulkan_sdk_path = None
        self.emscripten_path = None
        self.has_glfw = False

    def detect_all(self):
        """
        Detect all dependencies.
        
        Returns:
            tuple: (vulkan_sdk_path, emscripten_path, has_glfw)
        """
        self.vulkan_sdk_path = self.find_vulkan_sdk()
        self.emscripten_path = self.find_emscripten()
        self.has_glfw = self.find_glfw()
        
        return self.vulkan_sdk_path, self.emscripten_path, self.has_glfw

    @staticmethod
    def is_command_available(command):
        """
        Check if a command is available on the system.
        
        Args:
            command (str): The command to check.
            
        Returns:
            bool: True if the command is available, False otherwise.
        """
        return shutil.which(command) is not None

    def find_vulkan_sdk(self):
        """
        Try to find the Vulkan SDK installation.
        
        Returns:
            str or None: The path to the SDK or None if not found.
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

    def find_emscripten(self):
        """
        Try to find the Emscripten SDK installation.
        
        Returns:
            bool or str or None: True if emcc is in PATH, the path to the emsdk_env script,
                                or None if not found.
        """
        # Check if emcc is in PATH
        if self.is_command_available('emcc'):
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

    def find_glfw(self):
        """
        Try to find GLFW installation.
        
        Returns:
            bool: True if found, False otherwise.
        """
        # Check if pkg-config is available
        if self.is_command_available('pkg-config'):
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
