#!/usr/bin/env python3
"""Build the Markdown body of the sticky pull request benchmark comment.

Reads two Google Benchmark JSON files (base and PR, measured one after the
other on the same machine) and prints a comment with:

- the Patternia benchmarks, each marked slower / faster / within noise;
- the remaining implementations (std::visit, switch, ...) as control rows.
  Their code does not change between base and PR, so how far they move is a
  direct estimate of the measurement noise.

Usage:
  python scripts/bench_pr_comment.py --baseline base.json --current pr.json \
      --base-sha <sha> --head-sha <sha> --run-url <url> > comment.md
"""

from __future__ import annotations

import argparse
import re
import statistics
import sys
from pathlib import Path
from typing import List, Optional

sys.path.insert(0, str(Path(__file__).resolve().parent))
import bench_compare as bc  # noqa: E402

MARKER = "<!-- ptn-bench-compare -->"
MIN_THRESHOLD_PCT = 3.0
NO_NOISE_THRESHOLD_PCT = 5.0


def short_name(name: str) -> str:
    """BM_PatterniaPipe_VariantMixed/min_time:0.500/repeats:20 -> PatterniaPipe VariantMixed."""
    base = name.split("/", 1)[0]
    if base.startswith("BM_"):
        base = base[3:]
    return base.replace("_", " ")


def is_patternia(name: str) -> bool:
    return "Patternia" in name


def noise_estimate(controls: List[bc.BenchRow]) -> Optional[float]:
    deltas = [abs(r.delta_pct) for r in controls if bc._is_finite(r.delta_pct)]
    if len(deltas) < 3:
        return None
    return statistics.median(deltas)


def verdict(delta_pct: float, threshold: float) -> str:
    if not bc._is_finite(delta_pct):
        return "n/a"
    if delta_pct > threshold:
        return "slower"
    if delta_pct < -threshold:
        return "faster"
    return "within noise"


def table(rows: List[bc.BenchRow], threshold: Optional[float]) -> List[str]:
    header = "| Benchmark | Base (ns) | PR (ns) | Delta |"
    sep = "|---|---:|---:|---:|"
    if threshold is not None:
        header += " Verdict |"
        sep += "---|"
    lines = [header, sep]
    for r in rows:
        line = f"| {short_name(r.name)} | {r.baseline_mean_ns:.3f} | {r.current_mean_ns:.3f} | {r.delta_pct:+.1f}% |"
        if threshold is not None:
            line += f" {verdict(r.delta_pct, threshold)} |"
        lines.append(line)
    return lines


def build_comment(rows: List[bc.BenchRow], base_sha: str, head_sha: str, run_url: str) -> str:
    out = [
        MARKER,
        "### Benchmark comparison (informational)",
        "",
        f"Base `{base_sha[:7]}` vs PR `{head_sha[:7]}`, run one after the other on the same "
        "GitHub-hosted runner. This does not block merging.",
        "",
    ]

    if not rows:
        out += ["No overlapping benchmarks were found between base and PR.", ""]
    else:
        patternia = [r for r in rows if is_patternia(r.name)]
        controls = [r for r in rows if not is_patternia(r.name)]
        noise = noise_estimate(controls)
        threshold = max(MIN_THRESHOLD_PCT, 2.0 * noise) if noise is not None else NO_NOISE_THRESHOLD_PCT

        if noise is not None:
            out.append(
                f"**Noise estimate:** the {len(controls)} control rows below do not run Patternia code, so "
                f"they should not change between base and PR; they moved by a median of {noise:.1f}%. "
                f"Differences under {threshold:.1f}% are treated as noise."
            )
        else:
            out.append(
                "**Noise estimate:** not enough control rows; differences under "
                f"{threshold:.1f}% are treated as noise."
            )
        out.append("")

        if patternia:
            slower = sum(1 for r in patternia if verdict(r.delta_pct, threshold) == "slower")
            faster = sum(1 for r in patternia if verdict(r.delta_pct, threshold) == "faster")
            same = len(patternia) - slower - faster
            out.append(
                f"**Patternia:** {len(patternia)} benchmarks, {slower} slower, {faster} faster, "
                f"{same} within noise."
            )
            out.append("")
            out += table(patternia, threshold)
            out.append("")

        if controls:
            out.append("<details><summary>Control rows (not Patternia code)</summary>")
            out.append("")
            out += table(controls, None)
            out.append("")
            out.append("</details>")
            out.append("")

    out.append(
        f"Raw results and the comparison chart are attached to [this workflow run]({run_url}) "
        "as an artifact. Shared runners are noisy, so use this as a trend, not a measurement."
    )
    return "\n".join(out) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description="Build the PR benchmark comparison comment.")
    parser.add_argument("--baseline", required=True, help="Base result.json")
    parser.add_argument("--current", required=True, help="PR result.json")
    parser.add_argument("--include", default="", help="Regex: only include matching benchmark names")
    parser.add_argument("--base-sha", required=True)
    parser.add_argument("--head-sha", required=True)
    parser.add_argument("--run-url", required=True)
    args = parser.parse_args()

    baseline = bc._extract_aggregates(bc._load_json(Path(args.baseline)))
    current = bc._extract_aggregates(bc._load_json(Path(args.current)))
    rows = bc._to_rows(baseline, current, args.include.strip() or None)

    sys.stdout.write(build_comment(rows, args.base_sha, args.head_sha, args.run_url))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
