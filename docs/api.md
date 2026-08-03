# API overview

## Loading

- `load(path, options=...)` loads FBX, OBJ or MTL from a filesystem path.
- `loads(data, filename=..., options=...)` loads a contiguous bytes-like object.
- `LoadOptions` controls content, coordinate conversion and resource limits.
- `LoadError.kind` is a stable machine-readable failure category.

## Scene graph

`Scene` owns all native memory. Every child view retains the scene automatically.
Nodes expose hierarchy, transforms, typed attributes and arbitrary properties.
`Scene.elements` and `elements_of_type()` preserve uncommon ufbx elements.

## Geometry

`Mesh.vertices` and `faces` use logical vertices. Per-corner arrays are aligned
with `triangle_indices`, preserving UV and normal seams. Meshes also expose
half-edge topology, skinning, morph targets and subdivision metadata.

## Animation

`Scene.evaluate()` returns a new owned snapshot. Raw stacks, layers, curves,
keyframes and tangents are available for editing and conversion pipelines.

## Parametric geometry

NURBS curves and surfaces expose bases, knot vectors, control points and direct
evaluation through ufbx.

