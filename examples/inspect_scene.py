from __future__ import annotations

import argparse
from pathlib import Path

import pyfbx


def main() -> None:
    parser = argparse.ArgumentParser(description="Inspect an FBX scene hierarchy")
    parser.add_argument("scene", type=Path)
    args = parser.parse_args()

    scene = pyfbx.load(args.scene)
    print(scene)
    print(f"creator={scene.creator!r} unit={scene.unit_meters}m fps={scene.frames_per_second:g}")

    for node in pyfbx.walk(scene.root):
        depth = node.path.count("/") - 1
        suffix = ""
        if node.mesh is not None:
            suffix = f"  ({node.mesh.num_vertices} vertices, {node.mesh.num_triangles} triangles)"
        print(f"{'  ' * depth}{node.name or '<root>'} [{node.attribute_type}]{suffix}")


if __name__ == "__main__":
    main()

