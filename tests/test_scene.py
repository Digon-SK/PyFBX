from __future__ import annotations

import gc
from pathlib import Path

import pytest

import pyfbx

ASSET = Path(__file__).parent / "assets" / "cube_ascii.fbx"


def test_load_scene_and_navigate() -> None:
    scene = pyfbx.load(ASSET)

    assert scene.file_format == "fbx"
    assert scene.is_ascii is True
    assert scene.file_version == 5800
    assert len(scene) == len(scene.nodes)
    assert scene.root.is_root
    assert scene.meshes

    mesh = scene.meshes[0]
    assert mesh.num_vertices == len(mesh.vertices)
    assert mesh.num_faces == len(mesh.faces)
    assert mesh.num_triangles == len(mesh.triangles)
    assert len(mesh.vertex_positions) == mesh.num_indices
    assert len(mesh.triangle_indices) == mesh.num_triangles
    assert all(len(triangle) == 3 for triangle in mesh.triangles)


def test_child_keeps_native_scene_alive() -> None:
    scene = pyfbx.load(ASSET)
    mesh = scene.meshes[0]
    expected = mesh.vertices[0]

    del scene
    gc.collect()

    assert mesh.vertices[0] == expected
    assert mesh.instances[0].mesh is not None


def test_scene_lookup_and_iteration() -> None:
    scene = pyfbx.load(ASSET)
    named = next(node for node in scene.nodes if node.name)

    assert scene[named.name].element_id == named.element_id
    assert scene.find_node(named.name).element_id == named.element_id  # type: ignore[union-attr]
    assert scene[-1].element_id == scene.nodes[-1].element_id
    assert list(scene)
    with pytest.raises(KeyError):
        _ = scene["definitely absent"]


def test_material_values_are_python_values() -> None:
    material = pyfbx.load(ASSET).materials[0]

    assert len(material.fbx["diffuse_color"]["value"]) == 3
    assert isinstance(material.pbr["roughness"]["value"], float)


def test_loads_from_memory() -> None:
    data = ASSET.read_bytes()
    scene = pyfbx.loads(memoryview(data), filename="memory.fbx")

    assert scene.file_format == "fbx"
    assert len(scene.meshes) == 1


def test_options_validate_values() -> None:
    with pytest.raises(ValueError, match="greater than zero"):
        pyfbx.load(ASSET, options=pyfbx.LoadOptions(target_unit_meters=0.0))

    with pytest.raises(ValueError, match="memory_limit_bytes"):
        pyfbx.load(ASSET, options=pyfbx.LoadOptions(memory_limit_bytes=0))


def test_missing_file_raises_load_error(tmp_path: Path) -> None:
    with pytest.raises(pyfbx.LoadError) as caught:
        pyfbx.load(tmp_path / "missing.fbx")
    assert caught.value.kind == "file_not_found"


def test_views_have_stable_identity() -> None:
    scene = pyfbx.load(ASSET)

    assert scene.nodes[1] == scene.nodes[1]
    assert hash(scene.meshes[0]) == hash(scene.meshes[0])
    assert len({scene.nodes[1], scene.nodes[1]}) == 1


def test_evaluate_scene() -> None:
    scene = pyfbx.load(ASSET)
    evaluated = scene.evaluate(0.0)

    assert len(evaluated.nodes) == len(scene.nodes)
    assert evaluated.root.node_to_world == scene.root.node_to_world
