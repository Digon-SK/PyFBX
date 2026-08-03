from __future__ import annotations

from dataclasses import asdict, dataclass
from enum import Enum


class AxisSystem(str, Enum):
    """Common coordinate systems accepted by :class:`LoadOptions`."""

    ORIGINAL = "original"
    RIGHT_HANDED_Y_UP = "right_handed_y_up"
    RIGHT_HANDED_Z_UP = "right_handed_z_up"
    LEFT_HANDED_Y_UP = "left_handed_y_up"
    LEFT_HANDED_Z_UP = "left_handed_z_up"


@dataclass(frozen=True, slots=True)
class LoadOptions:
    """Safe, commonly useful subset of ``ufbx_load_opts``.

    External files are deliberately disabled by default because an untrusted FBX
    can reference arbitrary filesystem paths.
    """

    ignore_geometry: bool = False
    ignore_animation: bool = False
    ignore_embedded: bool = False
    evaluate_skinning: bool = False
    evaluate_caches: bool = False
    load_external_files: bool = False
    ignore_missing_external_files: bool = False
    generate_missing_normals: bool = True
    strict: bool = False
    retain_dom: bool = False
    target_axes: AxisSystem = AxisSystem.ORIGINAL
    target_unit_meters: float | None = None
    node_depth_limit: int = 0
    memory_limit_bytes: int | None = None

    def _native_dict(self) -> dict[str, object]:
        values = asdict(self)
        values["target_axes"] = self.target_axes.value
        return values
