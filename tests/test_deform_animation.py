from pathlib import Path

import pytest

import pyfbx

ASSETS = Path(__file__).parent / "assets"


def test_skin_weights_and_clusters() -> None:
    scene = pyfbx.load(ASSETS / "skinned_binary.fbx")
    skin = scene.meshes[0].skin_deformers[0]

    assert skin.num_vertices == scene.meshes[0].num_vertices
    assert skin.clusters[0].bone is not None
    assert skin.weights(0)[0]["weight"] == pytest.approx(1.0)
    with pytest.raises(IndexError):
        skin.weights(skin.num_vertices)


def test_blend_shape_offsets() -> None:
    scene = pyfbx.load(ASSETS / "blend_shape_ascii.fbx")
    channel = scene.meshes[0].blend_deformers[0].channels[0]
    shape = channel.keyframes[0]["shape"]

    assert channel.name == "TopH"
    assert shape.num_offsets == len(shape.offsets)
    assert shape.offsets[0]["vertex"] >= 0


def test_animation_curves_and_evaluation() -> None:
    scene = pyfbx.load(ASSETS / "animation_ascii.fbx")
    take = scene.animations[0]
    curve = take.layers[0].curves[0]

    assert take.duration == pytest.approx(2.0)
    assert curve.keyframes
    assert curve.time_range[0] <= curve.time_range[1]
    assert curve.evaluate(curve.keyframes[0]["time"]) == pytest.approx(
        curve.keyframes[0]["value"]
    )

