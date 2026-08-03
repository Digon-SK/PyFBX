from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

import pyfbx


def _validate_mesh(mesh: pyfbx.Mesh) -> dict[str, int]:
    vertices = mesh.vertices
    positions = mesh.vertex_positions
    faces = mesh.faces
    triangles = mesh.triangles
    triangle_indices = mesh.triangle_indices

    assert len(vertices) == mesh.num_vertices
    assert len(positions) == mesh.num_indices
    assert len(faces) == mesh.num_faces
    assert sum(len(face) for face in faces) == mesh.num_indices
    assert len(triangles) == mesh.num_triangles
    assert len(triangle_indices) == mesh.num_triangles
    assert all(len(triangle) == 3 for triangle in triangles)
    assert all(len(triangle) == 3 for triangle in triangle_indices)

    for attribute in (mesh.normals, mesh.uvs, mesh.colors):
        if attribute is not None:
            assert len(attribute) == mesh.num_indices

    if mesh.face_material_indices is not None:
        assert len(mesh.face_material_indices) == mesh.num_faces

    assert len(mesh.topology) == mesh.num_indices
    assert isinstance(mesh.properties, tuple)
    assert mesh.subdivision["preview_levels"] >= 0
    assert mesh.subdivision["render_levels"] >= 0
    assert isinstance(mesh.subdivision["evaluated"], bool)

    if mesh.num_faces:
        assert len(mesh.weighted_face_normal(0)) == 3

    for skin in mesh.skin_deformers:
        assert skin.num_vertices == mesh.num_vertices
        for cluster in skin.clusters:
            assert len(cluster.geometry_to_bone) == 4
            assert all(0 <= vertex < mesh.num_vertices for vertex, _weight in cluster.weights)

        if skin.num_vertices:
            sample_indices = {0, skin.num_vertices // 2, skin.num_vertices - 1}
            for vertex_index in sample_indices:
                for weight in skin.weights(vertex_index):
                    assert isinstance(weight["cluster"].name, str)
                    assert 0.0 <= weight["weight"] <= 1.0

    for deformer in mesh.blend_deformers:
        for channel in deformer.channels:
            for keyframe in channel.keyframes:
                shape = keyframe["shape"]
                assert shape.num_offsets == len(shape.offsets)

    return {
        "vertices": mesh.num_vertices,
        "indices": mesh.num_indices,
        "faces": mesh.num_faces,
        "triangles": mesh.num_triangles,
        "skins": len(mesh.skin_deformers),
        "blends": len(mesh.blend_deformers),
    }


def probe(path: Path) -> dict[str, Any]:
    scene = pyfbx.load(path)

    assert scene.file_format == "fbx"
    assert scene.root.is_root
    assert len(scene) == len(scene.nodes)
    assert tuple(scene) == scene.nodes
    assert all(node.path for node in scene.nodes)

    for node in scene.nodes:
        assert len(node.node_to_world) == 4
        assert len(node.geometry_to_world) == 4
        assert len(node.local_transform["translation"]) == 3
        assert len(node.geometry_transform["translation"]) == 3
        assert isinstance(node.properties, tuple)
        if node.parent is not None:
            assert node in node.parent.children

    meshes = [_validate_mesh(mesh) for mesh in scene.meshes]

    for material in scene.materials:
        assert isinstance(material.pbr, dict)
        assert isinstance(material.fbx, dict)
        assert isinstance(material.properties, tuple)

    for texture in scene.textures:
        assert isinstance(texture.filename, str)
        assert isinstance(texture.relative_filename, str)
        embedded_content = texture.embedded_content
        assert embedded_content is None or isinstance(embedded_content, bytes)

    for animation in scene.animations:
        assert animation.duration >= 0.0
        assert animation.time_begin <= animation.time_end
        for layer in animation.layers:
            for curve in layer.curves:
                assert curve.time_range[0] <= curve.time_range[1]
                assert curve.value_range[0] <= curve.value_range[1]
                if curve.keyframes:
                    curve.evaluate(curve.keyframes[0]["time"])

        evaluated = scene.evaluate(
            (animation.time_begin + animation.time_end) * 0.5,
            animation,
        )
        assert len(evaluated.nodes) == len(scene.nodes)

    return {
        "format": scene.file_format,
        "version": scene.file_version,
        "ascii": scene.is_ascii,
        "nodes": len(scene.nodes),
        "meshes": meshes,
        "materials": len(scene.materials),
        "textures": len(scene.textures),
        "animations": len(scene.animations),
        "curves": len(scene.animation_curves),
        "bones": len(scene.bones),
        "warnings": len(scene.warnings),
    }


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit("usage: _corpus_probe.py FILE.fbx")
    print(json.dumps(probe(Path(sys.argv[1])), sort_keys=True))


if __name__ == "__main__":
    main()
