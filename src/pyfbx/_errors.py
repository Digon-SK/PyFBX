from __future__ import annotations


class PyFbxError(RuntimeError):
    """Base class for errors raised by :mod:`pyfbx`."""


class LoadError(PyFbxError):
    """A scene could not be loaded or evaluated."""

    def __init__(self, message: str, *, kind: str = "unknown") -> None:
        super().__init__(message)
        self.kind = kind

