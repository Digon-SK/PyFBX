"""Pythonic bindings for the fast, single-file :mod:`ufbx` scene loader."""

from ._api import as_numpy, find, load, loads, walk
from ._native import (
    AnimationStack,
    Bone,
    Camera,
    Light,
    LoadError,
    Material,
    Mesh,
    Node,
    Property,
    Scene,
    Texture,
    __ufbx_version__,
)
from ._options import AxisSystem, LoadOptions

__all__ = [
    "AnimationStack",
    "AxisSystem",
    "Bone",
    "Camera",
    "Light",
    "LoadError",
    "LoadOptions",
    "Material",
    "Mesh",
    "Node",
    "Property",
    "Scene",
    "Texture",
    "__ufbx_version__",
    "as_numpy",
    "find",
    "load",
    "loads",
    "walk",
]

__version__ = "0.1.0"
