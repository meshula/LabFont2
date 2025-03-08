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

import argparse
from config.configuration_manager import ConfigurationManager


def main():
    """Main function to configure the build environment."""
    # Parse command-line arguments
    parser = argparse.ArgumentParser(description="Configure LabFont2 build environment")
    parser.add_argument("--xcode", action="store_true", help="Use Xcode generator instead of make")
    parser.add_argument("--build-dir", type=str, default="build", help="Directory to place the build scripts")
    args = parser.parse_args()
    
    # Create configuration manager with parsed arguments
    config_manager = ConfigurationManager(use_xcode=args.xcode, build_dir=args.build_dir)
    config_manager.configure()


if __name__ == "__main__":
    main()
