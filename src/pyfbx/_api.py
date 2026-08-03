from __future__ import annotations

import importlib
import os
from collections.abc import Callable, Iterator
from typing import Any, TypeVar

from . import _native
from ._options import LoadOptions

PathLike = str | os.PathLike[str]
T = TypeVar("T")


def load(path: PathLike, *, options: LoadOptions | None = None) -> _native.Scene:
    """Load an FBX (or OBJ/MTL) scene from *path*.

    The returned scene owns all native memory. Nodes, meshes, materials and
    textures retain that ownership automatically, so child objects remain valid
    even when the original ``Scene`` variable goes out of scope.
    """

    source = os.fspath(path)
    if not isinstance(source, str):
        source = os.fsdecode(source)
    return _native.load(source, (options or LoadOptions())._native_dict())


def loads(
    data: bytes | bytearray | memoryview,
    *,
    filename: PathLike = "scene.fbx",
    options: LoadOptions | None = None,
) -> _native.Scene:
    """Load a scene from a bytes-like object.

    ``filename`` is only a format/path hint; no main file is read from disk.
    """

    hint = os.fsdecode(os.fspath(filename))
    return _native.loads(data, hint, (options or LoadOptions())._native_dict())


def walk(node: _native.Node) -> Iterator[_native.Node]:
    """Yield *node* and all descendants depth-first without recursion."""

    pending = [node]
    while pending:
        current = pending.pop()
        yield current
        pending.extend(reversed(current.children))


def as_numpy(mesh: _native.Mesh) -> dict[str, Any]:
    """Copy a mesh into NumPy arrays.

    NumPy is optional. Faces are represented by the triangulated corner index
    stream so positions, normals and UV seams stay aligned.
    """

    try:
        np = importlib.import_module("numpy")
    except ImportError as exc:  # pragma: no cover - depends on optional extra
        raise ImportError("as_numpy() requires: pip install pyfbx-ufbx[numpy]") from exc

    result: dict[str, Any] = {
        "positions": np.asarray(mesh.vertex_positions, dtype=np.float64),
        "triangles": np.asarray(mesh.triangle_indices, dtype=np.uint32),
    }
    if mesh.normals is not None:
        result["normals"] = np.asarray(mesh.normals, dtype=np.float64)
    if mesh.uvs is not None:
        result["uvs"] = np.asarray(mesh.uvs, dtype=np.float64)
    if mesh.colors is not None:
        result["colors"] = np.asarray(mesh.colors, dtype=np.float64)
    return result


def find(
    scene: _native.Scene,
    predicate: Callable[[_native.Node], bool],
) -> list[_native.Node]:
    """Return every scene node for which ``predicate(node)`` is true."""

    return [node for node in scene.nodes if predicate(node)]
