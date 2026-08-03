from pathlib import Path

import pytest

import pyfbx

ASSETS = Path(__file__).parent / "assets"


def test_nurbs_curve_control_points_and_evaluation() -> None:
    scene = pyfbx.load(ASSETS / "nurbs_curve_ascii.fbx")
    curve = scene.nurbs_curves[0]
    parameter = curve.basis["parameter_range"][0]
    point = curve.evaluate(parameter)

    assert curve.basis["valid"]
    assert curve.control_points
    assert point["valid"]
    assert len(point["derivative"]) == 3


def test_nurbs_surface_control_grid_and_evaluation() -> None:
    scene = pyfbx.load(ASSETS / "nurbs_surface_ascii.fbx")
    surface = scene.nurbs_surfaces[0]
    dimensions = surface.control_point_dimensions
    point = surface.evaluate(
        surface.basis_u["parameter_range"][0],
        surface.basis_v["parameter_range"][0],
    )

    assert dimensions[0] * dimensions[1] == len(surface.control_points)
    assert point["valid"]


def test_generic_elements_cover_constraints() -> None:
    scene = pyfbx.load(ASSETS / "constraints_ascii.fbx")
    constraints = scene.elements_of_type("constraint")

    assert constraints
    assert constraints[0].type == "constraint"
    assert constraints[0].properties


def test_mesh_topology_and_face_normal() -> None:
    mesh = pyfbx.load(ASSETS / "cube_ascii.fbx").meshes[0]

    assert len(mesh.topology) == mesh.num_indices
    assert mesh.topology[0]["next"] >= 0
    assert isinstance(mesh.edges, tuple)  # FBX may legitimately omit explicit edge data.
    assert any(abs(component) > 0.0 for component in mesh.weighted_face_normal(0))
    with pytest.raises(IndexError):
        mesh.weighted_face_normal(mesh.num_faces)
