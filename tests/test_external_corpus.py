from __future__ import annotations

import json
import os
import subprocess
import sys
from pathlib import Path
from typing import Any

import pytest

CORPUS_ENV = "PYFBX_TEST_CORPUS"
PROBE = Path(__file__).with_name("_corpus_probe.py")
_configured_path = os.environ.get(CORPUS_ENV)
CORPUS_ROOT = Path(_configured_path).expanduser().resolve() if _configured_path else None
CORPUS_FILES = (
    tuple(sorted(CORPUS_ROOT.rglob("*.fbx")))
    if CORPUS_ROOT is not None and CORPUS_ROOT.is_dir()
    else ()
)


def _file_id(path: Path | None) -> str:
    if path is None or CORPUS_ROOT is None:
        return "not-configured"
    return path.relative_to(CORPUS_ROOT).as_posix()


def _probe(path: Path) -> dict[str, Any]:
    result = subprocess.run(
        [sys.executable, str(PROBE), str(path)],
        check=False,
        capture_output=True,
        text=True,
        timeout=60,
    )
    assert result.returncode == 0, (
        f"corpus probe failed for {_file_id(path)} with exit code {result.returncode}\n"
        f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}"
    )
    return json.loads(result.stdout)


def test_external_corpus_configuration() -> None:
    if CORPUS_ROOT is None:
        pytest.skip(f"set {CORPUS_ENV} to run external FBX compatibility tests")
    assert CORPUS_ROOT.is_dir(), f"{CORPUS_ENV} is not a directory: {CORPUS_ROOT}"
    assert CORPUS_FILES, f"no FBX files found below {CORPUS_ROOT}"


@pytest.mark.parametrize("fbx_path", CORPUS_FILES or (None,), ids=_file_id)
def test_external_fbx_in_isolated_process(fbx_path: Path | None) -> None:
    if fbx_path is None:
        pytest.skip(f"set {CORPUS_ENV} to run external FBX compatibility tests")

    summary = _probe(fbx_path)
    assert summary["format"] == "fbx"
    assert summary["nodes"] > 0


def test_converted_ascii_and_binary_files_match() -> None:
    if CORPUS_ROOT is None:
        pytest.skip(f"set {CORPUS_ENV} to run external FBX compatibility tests")

    pairs = []
    for binary_path in CORPUS_ROOT.rglob("*.binary.fbx"):
        text_path = binary_path.with_name(binary_path.name.replace(".binary.fbx", ".text.fbx"))
        if text_path.is_file():
            pairs.append((binary_path, text_path))

    if not pairs:
        pytest.skip("the corpus has no *.binary.fbx/*.text.fbx pairs")

    for binary_path, text_path in pairs:
        binary_summary = _probe(binary_path)
        text_summary = _probe(text_path)
        assert binary_summary.pop("ascii") is False
        assert text_summary.pop("ascii") is True
        assert binary_summary == text_summary
