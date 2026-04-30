#!/usr/bin/env python3
"""Measure compile-time for patternia compile-time benchmarks.

Compiles each compile-time TU N times and reports median wall-clock
compilation time.  Used as a regression gate: if the median compile
time of any TU increases beyond a threshold the script exits non-zero.
"""

from __future__ import annotations

import argparse
import json
import os
import statistics
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BUILD_DIR = REPO_ROOT / "build_ct_bench"


@dataclass
class CTResult:
    target: str
    elapsed_sec: float


def _parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(
        description="compile-time benchmark measurement"
    )
    p.add_argument(
        "--build-dir", default=str(DEFAULT_BUILD_DIR),
        help="CMake build directory for compile-time benchmarks"
    )
    p.add_argument(
        "--target", default="ptn_bench_ct",
        help="CMake target to build (aggregate)"
    )
    p.add_argument(
        "--repeat", type=int, default=3,
        help="Number of timed builds per TU (median taken)"
    )
    p.add_argument(
        "--source-dir", default=str(REPO_ROOT),
        help="Source tree root"
    )
    p.add_argument(
        "--json-out", default=None,
        help="Write JSON report to this path"
    )
    p.add_argument(
        "--baseline-json", default=None,
        help="Baseline JSON report for comparison"
    )
    p.add_argument(
        "--fail-if-regress-pct", type=float, default=5.0,
        help="Fail if any TU regresses above this percent"
    )
    p.add_argument(
        "--fail-if-mean-regress-pct", type=float, default=3.0,
        help="Fail if mean regression exceeds this percent"
    )
    return p.parse_args()


def _configure(build_dir: str, source_dir: str) -> None:
    if not Path(build_dir, "CMakeCache.txt").exists():
        subprocess.run(
            [
                "cmake", "-S", source_dir, "-B", build_dir,
                "-DCMAKE_BUILD_TYPE=Release",
                "-DPTN_BUILD_TESTS=OFF",
                "-DPTN_BUILD_BENCHMARKS=ON",
                "-DPTN_SKIP_COMPILER_CHECK=ON",
            ],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )


def _full_clean(build_dir: str) -> None:
    """Remove all object files so each run is a full recompile."""
    subprocess.run(
        ["cmake", "--build", build_dir, "--target", "clean"],
        capture_output=True,
    )


def _timed_build(build_dir: str, target: str) -> float:
    """Run a single cmake --build and return elapsed wall seconds."""
    start = time.perf_counter()
    subprocess.run(
        ["cmake", "--build", build_dir, "--target", target, "--parallel"],
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return time.perf_counter() - start


def _measure_tu(build_dir: str, tu_target: str,
                repeat: int) -> List[float]:
    times: List[float] = []
    for _ in range(repeat):
        _full_clean(build_dir)
        times.append(_timed_build(build_dir, tu_target))
    return times


def _tu_to_obj_target(tu: str) -> str:
    """Convert aggregate target name to the underlying object library name."""
    return tu.replace("ptn_bench_ct_", "ptn_ct_")


def _list_tu_targets(build_dir: str) -> List[str]:
    """Find all ptn_ct_* object targets from the build system."""
    import re
    result = subprocess.run(
        ["cmake", "--build", build_dir, "--target", "help"],
        capture_output=True,
        text=True,
        check=True,
    )
    targets: List[str] = []
    for line in result.stdout.splitlines():
        m = re.match(r"^\s*\.\.\.\s*(ptn_ct_\S+)\s*$", line)
        if m:
            targets.append(m.group(1))
    return sorted(targets)


def _median(lst: List[float]) -> float:
    return statistics.median(lst)


def main() -> int:
    args = _parse_args()

    build_dir = args.build_dir
    source_dir = args.source_dir

    _configure(build_dir, source_dir)

    tu_targets = _list_tu_targets(build_dir)
    if not tu_targets:
        print("ERROR: no compile-time TU targets found", file=sys.stderr)
        return 2

    print(f"Found {len(tu_targets)} compile-time TU targets")

    results: Dict[str, Dict] = {}

    for target in tu_targets:
        print(f"  measuring {target} ...", end=" ", flush=True)
        times = _measure_tu(build_dir, target, args.repeat)
        med = _median(times)
        results[target] = {
            "median_sec": round(med, 3),
            "raw_sec": [round(t, 3) for t in times],
        }
        print(f"{med:.3f}s")

    report = {
        "targets": results,
        "build_dir": build_dir,
        "repeat": args.repeat,
    }

    if args.json_out:
        with open(args.json_out, "w", encoding="utf-8") as f:
            json.dump(report, f, indent=2)
        print(f"\nReport written to {args.json_out}")

    if args.baseline_json:
        with open(args.baseline_json, "r", encoding="utf-8") as f:
            baseline = json.load(f)
        baseline_targets = baseline.get("targets", {})

        print("\nCompile-time regression gate:")
        print(f"{'TU':<30} {'base(s)':>10} {'curr(s)':>10} {'delta%':>8}")
        print("-" * 60)

        deltas: List[float] = []
        failures = 0

        for target in sorted(results.keys()):
            cur = results[target]["median_sec"]
            base_data = baseline_targets.get(target)
            if base_data is None:
                print(f"{target:<30} {'--':>10} {cur:>10.3f}  (new)")
                continue

            base = base_data["median_sec"]
            delta = (cur - base) / base * 100.0 if base > 0 else 0.0
            deltas.append(delta)
            status = "FAIL" if delta > args.fail_if_regress_pct else "OK"
            if delta > args.fail_if_regress_pct:
                failures += 1

            print(
                f"{target:<30} {base:>10.3f} {cur:>10.3f} {delta:>+7.1f}%  {status}"
            )

        if deltas:
            mean_delta = statistics.mean(deltas)
            print(f"\nMean regression: {mean_delta:+.1f}%")

            if mean_delta > args.fail_if_mean_regress_pct:
                print(
                    f"FAIL: mean regression {mean_delta:+.1f}% exceeds "
                    f"threshold {args.fail_if_mean_regress_pct}%",
                    file=sys.stderr,
                )
                return 1

        if failures > 0:
            print(
                f"FAIL: {failures} TU(s) regressed above "
                f"{args.fail_if_regress_pct}%",
                file=sys.stderr,
            )
            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
