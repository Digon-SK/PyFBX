from pathlib import Path

import pyfbx

ASSETS = Path(__file__).parent / "assets"


def test_camera_and_light_entities() -> None:
    scene = pyfbx.load(ASSETS / "camera_light_ascii.fbx")

    camera = scene.cameras[0]
    assert camera.projection == "perspective"
    assert camera.field_of_view[0] > 0.0
    assert next(node for node in scene.nodes if node.camera is not None).camera is not None

    light = scene.lights[0]
    assert light.type == "directional"
    assert light.intensity > 0.0
    assert next(node for node in scene.nodes if node.light is not None).light is not None


def test_bones_and_custom_properties() -> None:
    scene = pyfbx.load(ASSETS / "bone_binary.fbx")

    assert len(scene.bones) == 5
    assert scene.bones[0].radius > 0.0
    bone_node = next(node for node in scene.nodes if node.bone is not None)
    assert bone_node.bone is not None
    assert bone_node.properties
    assert bone_node.find_property("Lcl Translation") is not None

