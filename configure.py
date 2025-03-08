#!/usr/bin/env python3
"""
LabFont2 Configuration Script

This script helps configure the build environment for LabFont2 by:
1. Detecting required dependencies (Vulkan SDK, Emscripten, GLFW)
2. Generating appropriate build scripts
3. Providing guidance on missing dependencies

The implementation is organized in a modular way, with separate classes for
different aspects of the configuration process.
"""

from config.configuration_manager import ConfigurationManager


def main():
    """Main function to configure the build environment."""
    config_manager = ConfigurationManager()
    config_manager.configure()


if __name__ == "__main__":
    main()
