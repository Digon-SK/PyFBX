# Contributing

## Local setup

```bash
python -m venv .venv
python -m pip install -e ".[dev]"
pytest -q
```

The native layer is split by domain under `src/native`. Views never own raw ufbx
pointers directly: each view must retain the `scene_owner` that owns the memory.

Before opening a pull request, run:

```bash
ruff check src tests examples benchmarks
mypy src/pyfbx
pytest -q
```

New bindings should include a compact fixture when an existing asset does not
exercise the behavior. Keep the public API in `src/pyfbx/__init__.py` and update
`src/pyfbx/_native.pyi` alongside native changes.

## External FBX corpora

Large or third-party FBX collections are tested without adding them to the
repository. Set `PYFBX_TEST_CORPUS` to a directory and run the opt-in suite:

```powershell
$env:PYFBX_TEST_CORPUS = "C:\path\to\fbx-corpus"
pytest -q tests/test_external_corpus.py
```

```bash
PYFBX_TEST_CORPUS=/path/to/fbx-corpus pytest -q tests/test_external_corpus.py
```

Each FBX runs in its own subprocess with a timeout, so a native crash or hang is
reported against the exact file. Matching `*.binary.fbx` and `*.text.fbx` files
are also checked for equivalent scene summaries.

## Releases

The version in `pyproject.toml` and the GitHub tag must match. Publishing a
GitHub release builds wheels for CPython 3.10 through 3.14 on Linux, macOS and
Windows, builds an sdist, and attaches every artifact to the release.

PyPI publication is deliberately opt-in. Configure the `pypi` trusted publisher
and environment first, then set the repository variable `PYPI_PUBLISH=true`.
