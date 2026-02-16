#!/usr/bin/env python3
"""Stage benchmark JSON files into a standard repo-local location.

Target layout:
- bench/results/baseline/result.json
- bench/results/current/result.json

After staging, run:
  py -3 scripts/bench_compare.py
"""

from __future__ import annotations

import argparse
import shutil
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DEST = REPO_ROOT / "bench" / "results"


def _resolve_result_path(raw: str) -> Path:
    p = Path(raw).expanduser().resolve()
    if p.is_dir():
        p = p / "result.json"
    return p


def _copy(src: Path, dst: Path) -> None:
    if not src.exists():
        raise SystemExit(f"source not found: {src}")
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)


def main() -> int:
    parser = argparse.ArgumentParser(description="Stage benchmark result.json files for local compare script.")
    parser.add_argument("--baseline", required=True, help="Baseline file path or containing directory")
    parser.add_argument("--current", required=True, help="Current file path or containing directory")
    parser.add_argument("--dest", default=str(DEFAULT_DEST), help="Destination root (default: bench/results)")
    args = parser.parse_args()

    baseline_src = _resolve_result_path(args.baseline)
    current_src = _resolve_result_path(args.current)

    dest_root = Path(args.dest).expanduser().resolve()
    baseline_dst = dest_root / "baseline" / "result.json"
    current_dst = dest_root / "current" / "result.json"

    _copy(baseline_src, baseline_dst)
    _copy(current_src, current_dst)

    print(f"Staged baseline: {baseline_dst}")
    print(f"Staged current:  {current_dst}")
    print("Now run: py -3 scripts/bench_compare.py")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
