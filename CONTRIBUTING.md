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

