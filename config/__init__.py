"""
LabFont2 Configuration Package

This package contains modules for configuring the build environment for LabFont2.
"""

from .dependency_detector import DependencyDetector
from .script_generator import ScriptGenerator
from .readme_updater import ReadmeUpdater
from .cmake_updater import CMakeUpdater
from .configuration_manager import ConfigurationManager

__all__ = [
    'DependencyDetector',
    'ScriptGenerator',
    'ReadmeUpdater',
    'CMakeUpdater',
    'ConfigurationManager',
]
