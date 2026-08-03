from __future__ import annotations

import argparse
import statistics
import time
from pathlib import Path

import pyfbx


def main() -> None:
    parser = argparse.ArgumentParser(description="Benchmark repeated ufbx scene loads")
    parser.add_argument("scene", type=Path)
    parser.add_argument("--iterations", type=int, default=20)
    args = parser.parse_args()
    if args.iterations < 1:
        parser.error("--iterations must be positive")

    data = args.scene.read_bytes()
    samples: list[float] = []
    for _ in range(args.iterations):
        started = time.perf_counter()
        scene = pyfbx.loads(data, filename=args.scene.name)
        samples.append(time.perf_counter() - started)
        del scene

    median = statistics.median(samples)
    print(f"iterations={len(samples)} median={median * 1000:.3f}ms")
    print(f"min={min(samples) * 1000:.3f}ms max={max(samples) * 1000:.3f}ms")


if __name__ == "__main__":
    main()

