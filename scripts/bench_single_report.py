#!/usr/bin/env python3
"""Visualize a single Google Benchmark JSON across implementations.

Use case:
- One benchmark JSON contains multiple implementation styles
  (Patternia / if-else / switch / std::visit).
- We want per-scenario comparison in a single report.
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Tuple


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_INPUT = REPO_ROOT / "build" / "variant_all.json"
DEFAULT_OUTDIR = REPO_ROOT / "build" / "bench" / "single"


@dataclass
class BenchPoint:
    base_name: str
    impl: str
    scenario: str
    mean_ns: float
    cv_pct: Optional[float]
    median_ns: Optional[float]
    stddev_ns: Optional[float]


def _load_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def _base_name_from_entry(entry: dict) -> Optional[str]:
    run_name = entry.get("run_name")
    if isinstance(run_name, str) and run_name:
        return run_name

    name = entry.get("name")
    if not isinstance(name, str) or not name:
        return None

    # Fallback for payloads where only "name" is present.
    if name.endswith("_mean"):
        return name[: -len("_mean")]
    if name.endswith("_median"):
        return name[: -len("_median")]
    if name.endswith("_stddev"):
        return name[: -len("_stddev")]
    if name.endswith("_cv"):
        return name[: -len("_cv")]
    return name


def _extract_metrics(payload: dict) -> Dict[str, Dict[str, float]]:
    out: Dict[str, Dict[str, float]] = {}
    for entry in payload.get("benchmarks", []):
        base = _base_name_from_entry(entry)
        if base is None:
            continue

        slot = out.setdefault(base, {})
        agg = entry.get("aggregate_name")
        cpu_time = entry.get("cpu_time")
        if cpu_time is None:
            continue

        if isinstance(agg, str) and agg:
            slot[agg] = float(cpu_time)
        elif "mean" not in slot:
            # Non-aggregate fallback.
            slot["mean"] = float(cpu_time)
    return out


def _split_impl_and_scenario(base_name: str) -> Tuple[str, str]:
    # Strip benchmark parameter suffix: ".../min_time...".
    core = base_name.split("/", 1)[0]

    if core.startswith("BM_"):
        core = core[3:]

    parts = core.split("_", 1)
    if len(parts) == 1:
        return "Unknown", core
    return parts[0], parts[1]


def _default_impl_order(impls: Iterable[str]) -> List[str]:
    preferred = ["Patternia", "IfElse", "Switch", "SwitchIndex", "StdVisit"]
    found = set(impls)
    ordered = [x for x in preferred if x in found]
    others = sorted(found - set(ordered))
    return ordered + others


def _to_points(
    metrics: Dict[str, Dict[str, float]],
    include_regex: Optional[str],
) -> List[BenchPoint]:
    pat = re.compile(include_regex) if include_regex else None
    points: List[BenchPoint] = []

    for base_name, m in sorted(metrics.items()):
        if pat and not pat.search(base_name):
            continue
        mean_ns = m.get("mean")
        if mean_ns is None:
            continue
        impl, scenario = _split_impl_and_scenario(base_name)
        cv = m.get("cv")
        points.append(
            BenchPoint(
                base_name=base_name,
                impl=impl,
                scenario=scenario,
                mean_ns=mean_ns,
                cv_pct=(cv * 100.0 if cv is not None else None),
                median_ns=m.get("median"),
                stddev_ns=m.get("stddev"),
            )
        )
    return points


def _to_nested(points: List[BenchPoint]) -> Dict[str, Dict[str, BenchPoint]]:
    nested: Dict[str, Dict[str, BenchPoint]] = {}
    for p in points:
        nested.setdefault(p.scenario, {})[p.impl] = p
    return nested


def _save_csv(points: List[BenchPoint], path: Path) -> None:
    with path.open("w", encoding="utf-8", newline="") as f:
        w = csv.writer(f)
        w.writerow(
            [
                "base_name",
                "scenario",
                "impl",
                "mean_ns",
                "median_ns",
                "stddev_ns",
                "cv_pct",
            ]
        )
        for p in points:
            w.writerow(
                [
                    p.base_name,
                    p.scenario,
                    p.impl,
                    f"{p.mean_ns:.6f}",
                    "" if p.median_ns is None else f"{p.median_ns:.6f}",
                    "" if p.stddev_ns is None else f"{p.stddev_ns:.6f}",
                    "" if p.cv_pct is None else f"{p.cv_pct:.4f}",
                ]
            )


def _save_markdown(
    nested: Dict[str, Dict[str, BenchPoint]],
    scenarios: List[str],
    impl_order: List[str],
    json_path: Path,
    out_md: Path,
) -> None:
    lines: List[str] = [
        "# Single Benchmark Report",
        "",
        f"- Source: `{json_path}`",
        "",
    ]

    for scenario in scenarios:
        row = nested[scenario]
        fastest = min(row.values(), key=lambda x: x.mean_ns)
        lines.append(f"## {scenario}")
        lines.append("")
        lines.append("| Impl | Mean (ns) | Delta vs fastest | CV % |")
        lines.append("|---|---:|---:|---:|")

        for impl in impl_order:
            p = row.get(impl)
            if p is None:
                continue
            delta = (
                ((p.mean_ns - fastest.mean_ns) / fastest.mean_ns) * 100.0
                if fastest.mean_ns > 0
                else math.nan
            )
            cv = "-" if p.cv_pct is None else f"{p.cv_pct:.2f}"
            delta_s = "+0.00%" if p is fastest else f"{delta:+.2f}%"
            lines.append(f"| {impl} | {p.mean_ns:.3f} | {delta_s} | {cv} |")
        lines.append("")

    out_md.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _plot(
    nested: Dict[str, Dict[str, BenchPoint]],
    scenarios: List[str],
    impl_order: List[str],
    out_png: Path,
    title: str,
) -> None:
    try:
        import matplotlib.pyplot as plt
    except Exception as exc:  # pragma: no cover
        raise SystemExit(
            "matplotlib is required. Install: py -3 -m pip install matplotlib"
        ) from exc

    x = list(range(len(scenarios)))
    impls = [i for i in impl_order if any(i in nested[s] for s in scenarios)]
    if not impls:
        raise SystemExit("No implementations to plot.")

    width = 0.78 / max(1, len(impls))
    fig_w = max(12.0, len(scenarios) * 1.55 + 5.2)
    fig_h = 8.6

    plt.style.use("seaborn-v0_8-whitegrid")
    plt.rcParams.update(
        {
            "font.size": 10,
            "axes.titlesize": 13,
            "axes.labelsize": 11,
            "legend.fontsize": 10,
        }
    )

    fig, (ax1, ax2) = plt.subplots(
        2,
        1,
        figsize=(fig_w, fig_h),
        dpi=220,
        gridspec_kw={"height_ratios": [2.15, 1.1]},
    )

    fig.patch.set_facecolor("#f8fafc")
    for ax in (ax1, ax2):
        ax.set_facecolor("#ffffff")

    color_map = {
        "Patternia": "#0ea5e9",
        "IfElse": "#f59e0b",
        "Switch": "#10b981",
        "SwitchIndex": "#22c55e",
        "StdVisit": "#8b5cf6",
    }
    fallback_palette = ["#64748b", "#ef4444", "#14b8a6", "#f97316", "#6366f1"]

    means_by_impl: Dict[str, List[float]] = {}
    for idx, impl in enumerate(impls):
        means = [
            nested[s][impl].mean_ns if impl in nested[s] else float("nan")
            for s in scenarios
        ]
        means_by_impl[impl] = means
        offset = (idx - (len(impls) - 1) / 2.0) * width
        color = color_map.get(impl, fallback_palette[idx % len(fallback_palette)])
        ax1.bar(
            [i + offset for i in x],
            means,
            width=width,
            label=impl,
            color=color,
            edgecolor="#0f172a",
            linewidth=0.25,
            alpha=0.96,
            zorder=3,
        )

    # Relative slowdown (%) vs fastest implementation per scenario.
    for idx, impl in enumerate(impls):
        rel = []
        for si, sc in enumerate(scenarios):
            row = nested[sc]
            fastest = min(v.mean_ns for v in row.values())
            v = means_by_impl[impl][si]
            if math.isnan(v):
                rel.append(float("nan"))
            else:
                rel.append((v / fastest - 1.0) * 100.0 if fastest > 0 else float("nan"))

        offset = (idx - (len(impls) - 1) / 2.0) * width
        color = color_map.get(impl, fallback_palette[idx % len(fallback_palette)])
        ax2.bar(
            [i + offset for i in x],
            rel,
            width=width,
            color=color,
            edgecolor="#0f172a",
            linewidth=0.2,
            alpha=0.92,
            zorder=3,
        )

    # Mark fastest implementation in each scenario on top panel.
    for si, sc in enumerate(scenarios):
        row = nested[sc]
        fastest_impl = min(row.items(), key=lambda kv: kv[1].mean_ns)[0]
        fi = impls.index(fastest_impl)
        offset = (fi - (len(impls) - 1) / 2.0) * width
        y = row[fastest_impl].mean_ns
        ax1.scatter(
            [si + offset],
            [y],
            marker="D",
            s=20,
            color="#111827",
            zorder=4,
        )

    def _label(s: str) -> str:
        return re.sub(r"(?<!^)(?=[A-Z])", " ", s).replace("_", " ")

    labels = [_label(s) for s in scenarios]

    ax1.set_title(f"{title} - Mean CPU Time (lower is better)", fontweight="bold", pad=10)
    ax1.set_ylabel("ns")
    ax1.grid(axis="y", alpha=0.23, zorder=0)
    ax1.set_xticks(x)
    ax1.set_xticklabels(labels, rotation=14, ha="right")

    ax2.set_title("Relative Slowdown vs Fastest in Scenario (%)", fontweight="bold", pad=8)
    ax2.set_ylabel("%")
    ax2.axhline(0.0, color="#111827", linewidth=0.8, alpha=0.7)
    ax2.grid(axis="y", alpha=0.23, zorder=0)
    ax2.set_xticks(x)
    ax2.set_xticklabels(labels, rotation=14, ha="right")

    handles, legends = ax1.get_legend_handles_labels()
    fig.legend(
        handles,
        legends,
        loc="upper center",
        ncol=min(5, len(legends)),
        frameon=False,
        bbox_to_anchor=(0.5, 0.985),
    )
    fig.tight_layout(rect=(0, 0, 1, 0.93))
    fig.savefig(out_png, bbox_inches="tight")
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Visualize one benchmark JSON across implementations."
    )
    parser.add_argument("--input", default=str(DEFAULT_INPUT), help="Input benchmark JSON")
    parser.add_argument("--include", default="", help="Regex filter on benchmark name")
    parser.add_argument("--outdir", default=str(DEFAULT_OUTDIR), help="Output directory")
    parser.add_argument("--prefix", default="bench_single", help="Output file prefix")
    parser.add_argument("--title", default="Benchmark Single Report", help="Chart title")
    parser.add_argument(
        "--no-plot",
        action="store_true",
        help="Skip PNG generation and only write markdown/csv.",
    )
    args = parser.parse_args()

    input_path = Path(args.input).resolve()
    outdir = Path(args.outdir).resolve()
    outdir.mkdir(parents=True, exist_ok=True)

    payload = _load_json(input_path)
    metrics = _extract_metrics(payload)
    points = _to_points(metrics, args.include.strip() or None)
    if not points:
        raise SystemExit("No benchmark entries found after filtering.")

    nested = _to_nested(points)
    scenarios = sorted(nested.keys())
    impl_order = _default_impl_order(p.impl for p in points)

    out_md = outdir / f"{args.prefix}.md"
    out_csv = outdir / f"{args.prefix}.csv"
    out_png = outdir / f"{args.prefix}.png"

    _save_markdown(nested, scenarios, impl_order, input_path, out_md)
    _save_csv(points, out_csv)
    if not args.no_plot:
        _plot(nested, scenarios, impl_order, out_png, args.title)

    print(f"Saved markdown: {out_md}")
    print(f"Saved csv: {out_csv}")
    if not args.no_plot:
        print(f"Saved chart: {out_png}")


if __name__ == "__main__":
    main()
