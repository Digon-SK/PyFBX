from pathlib import Path

import pyfbx

ASSETS = Path(__file__).parent / "assets"


def test_constraints_expose_targets() -> None:
    constraint = pyfbx.load(ASSETS / "constraints_ascii.fbx").constraints[0]

    assert constraint.type == "position"
    assert constraint.active
    assert [target["node"].name for target in constraint.targets] == ["TargetA", "TargetB"]


def test_poses_expose_bone_matrices() -> None:
    pose = pyfbx.load(ASSETS / "poses_ascii.fbx").poses[0]

    assert pose.is_bind_pose
    assert pose.bones
    assert len(pose.bones[0]["bone_to_world"]) == 4


def test_audio_layers_and_embedded_content() -> None:
    scene = pyfbx.load(ASSETS / "audio_ascii.fbx")

    assert scene.audio_layers[0].clips[0].name == "plonk"
    assert scene.audio_clips[0].embedded_content


def test_retained_dom_tree() -> None:
    scene = pyfbx.load(
        ASSETS / "cube_ascii.fbx",
        options=pyfbx.LoadOptions(retain_dom=True),
    )

    assert scene.dom is not None
    assert scene.dom.children
    assert scene.dom.children[0].name == "FBXHeaderExtension"

