from __future__ import annotations

from pathlib import Path

import pytest

import pyfbx

ASSET = Path(__file__).parent / "assets" / "cube_ascii.fbx"


def test_walk_and_find_helpers() -> None:
    scene = pyfbx.load(ASSET)
    walked = list(pyfbx.walk(scene.root))

    assert walked[0].is_root
    assert len(walked) == len(scene.nodes)
    assert pyfbx.find(scene, lambda node: node.mesh is not None)


def test_numpy_conversion() -> None:
    np = pytest.importorskip("numpy")
    mesh = pyfbx.load(ASSET).meshes[0]
    arrays = pyfbx.as_numpy(mesh)

    assert arrays["positions"].shape == (mesh.num_indices, 3)
    assert arrays["triangles"].shape == (mesh.num_triangles, 3)
    assert arrays["triangles"].dtype == np.uint32

