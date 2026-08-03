"""Pythonic bindings for the fast, single-file :mod:`ufbx` scene loader."""

from ._api import as_numpy, find, load, loads, walk
from ._errors import LoadError, PyFbxError
from ._native import (
    AnimationCurve,
    AnimationLayer,
    AnimationStack,
    BlendChannel,
    BlendDeformer,
    BlendShape,
    Bone,
    Camera,
    Element,
    Light,
    Material,
    Mesh,
    Node,
    NurbsCurve,
    NurbsSurface,
    Property,
    Scene,
    SkinCluster,
    SkinDeformer,
    Texture,
    __ufbx_version__,
)
from ._options import AxisSystem, LoadOptions

__all__ = [
    "AnimationCurve",
    "AnimationLayer",
    "AnimationStack",
    "AxisSystem",
    "BlendChannel",
    "BlendDeformer",
    "BlendShape",
    "Bone",
    "Camera",
    "Element",
    "Light",
    "LoadError",
    "LoadOptions",
    "Material",
    "Mesh",
    "Node",
    "NurbsCurve",
    "NurbsSurface",
    "Property",
    "PyFbxError",
    "Scene",
    "SkinCluster",
    "SkinDeformer",
    "Texture",
    "__ufbx_version__",
    "as_numpy",
    "find",
    "load",
    "loads",
    "walk",
]

__version__ = "0.1.0"
