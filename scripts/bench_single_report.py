#!/usr/bin/env python3
"""Visualize a single Google Benchmark JSON/CSV across implementations.

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


@dataclass
class ScenarioSummary:
    scenario: str
    fastest_impl: str
    fastest_mean_ns: float
    patternia_impl: Optional[str]
    patternia_mean_ns: Optional[float]
    patternia_cv_pct: Optional[float]
    patternia_rank: Optional[int]
    patternia_delta_pct: Optional[float]
    patternia_ratio: Optional[float]
    status: str


PATTERNIA_IMPLS = {"Patternia", "PatterniaPipe"}


def _load_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def _display_path(path: Path) -> str:
    try:
        return path.resolve().relative_to(REPO_ROOT).as_posix()
    except ValueError:
        return str(path)


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
    preferred = ["Patternia", "PatterniaPipe", "IfElse", "Switch", "SwitchIndex", "StdVisit"]
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


def _points_from_csv(path: Path, include_regex: Optional[str]) -> List[BenchPoint]:
    pat = re.compile(include_regex) if include_regex else None
    points: List[BenchPoint] = []

    with path.open("r", encoding="utf-8", newline="") as f:
        for row in csv.DictReader(f):
            base_name = row.get("base_name", "")
            if pat and not pat.search(base_name):
                continue

            try:
                mean_ns = float(row["mean_ns"])
            except (KeyError, TypeError, ValueError):
                continue

            scenario = row.get("scenario", "")
            impl = row.get("impl", "")
            if not scenario or not impl:
                impl, scenario = _split_impl_and_scenario(base_name)

            def opt_float(name: str) -> Optional[float]:
                raw = row.get(name, "")
                if raw == "":
                    return None
                try:
                    return float(raw)
                except ValueError:
                    return None

            points.append(
                BenchPoint(
                    base_name=base_name,
                    impl=impl,
                    scenario=scenario,
                    mean_ns=mean_ns,
                    cv_pct=opt_float("cv_pct"),
                    median_ns=opt_float("median_ns"),
                    stddev_ns=opt_float("stddev_ns"),
                )
            )

    return points


def _to_nested(points: List[BenchPoint]) -> Dict[str, Dict[str, BenchPoint]]:
    nested: Dict[str, Dict[str, BenchPoint]] = {}
    for p in points:
        nested.setdefault(p.scenario, {})[p.impl] = p
    return nested


def _label(s: str) -> str:
    s = re.sub(r"(?<=[a-z0-9])(?=[A-Z])", " ", s)
    s = re.sub(r"(?<=[a-zA-Z])(?=\d)", " ", s)
    return s.strip().replace("_", " ")


def _patternia_best(row: Dict[str, BenchPoint]) -> Optional[BenchPoint]:
    return min(
        (p for impl, p in row.items() if impl in PATTERNIA_IMPLS),
        key=lambda p: p.mean_ns,
        default=None,
    )


def _status(delta_pct: Optional[float]) -> str:
    if delta_pct is None:
        return "missing"
    if delta_pct <= 0.01:
        return "fastest"
    if delta_pct <= 5.0:
        return "close"
    if delta_pct <= 20.0:
        return "watch"
    return "slow"


def _summaries(
    nested: Dict[str, Dict[str, BenchPoint]],
    scenarios: List[str],
) -> Dict[str, ScenarioSummary]:
    summaries: Dict[str, ScenarioSummary] = {}
    for scenario in scenarios:
        row = nested[scenario]
        ordered = sorted(row.values(), key=lambda p: p.mean_ns)
        fastest = ordered[0]
        pat = _patternia_best(row)

        rank = None
        delta_pct = None
        ratio = None
        if pat is not None and fastest.mean_ns > 0:
            rank = next(i for i, p in enumerate(ordered, start=1) if p is pat)
            ratio = pat.mean_ns / fastest.mean_ns
            delta_pct = (ratio - 1.0) * 100.0

        summaries[scenario] = ScenarioSummary(
            scenario=scenario,
            fastest_impl=fastest.impl,
            fastest_mean_ns=fastest.mean_ns,
            patternia_impl=None if pat is None else pat.impl,
            patternia_mean_ns=None if pat is None else pat.mean_ns,
            patternia_cv_pct=None if pat is None else pat.cv_pct,
            patternia_rank=rank,
            patternia_delta_pct=delta_pct,
            patternia_ratio=ratio,
            status=_status(delta_pct),
        )
    return summaries


def _scenario_order(summaries: Dict[str, ScenarioSummary]) -> List[str]:
    def key(scenario: str) -> Tuple[float, str]:
        summary = summaries[scenario]
        if summary.patternia_delta_pct is None:
            return (float("inf"), scenario)
        # Worst Patternia gaps first, then stable alphabetical order.
        return (-summary.patternia_delta_pct, scenario)

    return sorted(summaries, key=key)


def _fmt_opt(value: Optional[float], fmt: str) -> str:
    return "-" if value is None else format(value, fmt)


def _save_csv(
    points: List[BenchPoint],
    summaries: Dict[str, ScenarioSummary],
    path: Path,
) -> None:
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
                "is_patternia",
                "fastest_impl",
                "fastest_mean_ns",
                "impl_vs_fastest_pct",
                "impl_vs_patternia_pct",
                "patternia_impl",
                "patternia_mean_ns",
                "patternia_rank",
                "patternia_vs_fastest_pct",
                "patternia_status",
            ]
        )
        for p in points:
            summary = summaries[p.scenario]
            impl_delta = (
                (p.mean_ns / summary.fastest_mean_ns - 1.0) * 100.0
                if summary.fastest_mean_ns > 0
                else math.nan
            )
            impl_vs_pat = (
                (p.mean_ns / summary.patternia_mean_ns - 1.0) * 100.0
                if summary.patternia_mean_ns and summary.patternia_mean_ns > 0
                else None
            )
            w.writerow(
                [
                    p.base_name,
                    p.scenario,
                    p.impl,
                    f"{p.mean_ns:.6f}",
                    "" if p.median_ns is None else f"{p.median_ns:.6f}",
                    "" if p.stddev_ns is None else f"{p.stddev_ns:.6f}",
                    "" if p.cv_pct is None else f"{p.cv_pct:.4f}",
                    "yes" if p.impl in PATTERNIA_IMPLS else "no",
                    summary.fastest_impl,
                    f"{summary.fastest_mean_ns:.6f}",
                    f"{impl_delta:.4f}",
                    "" if impl_vs_pat is None else f"{impl_vs_pat:.4f}",
                    "" if summary.patternia_impl is None else summary.patternia_impl,
                    "" if summary.patternia_mean_ns is None else f"{summary.patternia_mean_ns:.6f}",
                    "" if summary.patternia_rank is None else str(summary.patternia_rank),
                    "" if summary.patternia_delta_pct is None else f"{summary.patternia_delta_pct:.4f}",
                    summary.status,
                ]
            )


def _save_markdown(
    nested: Dict[str, Dict[str, BenchPoint]],
    scenarios: List[str],
    impl_order: List[str],
    summaries: Dict[str, ScenarioSummary],
    json_path: Path,
    out_md: Path,
) -> None:
    wins = sum(1 for s in scenarios if summaries[s].status == "fastest")
    covered = [summaries[s] for s in scenarios if summaries[s].patternia_delta_pct is not None]
    avg_gap = sum(s.patternia_delta_pct for s in covered if s.patternia_delta_pct is not None) / max(1, len(covered))
    worst = max(
        covered,
        key=lambda s: s.patternia_delta_pct if s.patternia_delta_pct is not None else -1.0,
        default=None,
    )

    lines: List[str] = [
        "# Patternia Benchmark Report",
        "",
        f"- Source: `{_display_path(json_path)}`",
        f"- Scenarios: `{len(scenarios)}`",
        f"- Patternia fastest: `{wins}/{len(scenarios)}`",
        f"- Average Patternia gap vs fastest: `{avg_gap:+.2f}%`",
        ""
        if worst is None
        else f"- Largest Patternia gap: `{worst.scenario}` `{worst.patternia_delta_pct:+.2f}%` vs `{worst.fastest_impl}`",
        "",
        "## Patternia Focus",
        "",
        "| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |",
        "|---|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]

    for scenario in scenarios:
        summary = summaries[scenario]
        rank = (
            "-"
            if summary.patternia_rank is None
            else f"{summary.patternia_rank}/{len(nested[scenario])}"
        )
        gap = (
            "-"
            if summary.patternia_delta_pct is None
            else f"{summary.patternia_delta_pct:+.2f}%"
        )
        lines.append(
            f"| {scenario} | {summary.patternia_impl or '-'} | {rank} | "
            f"{_fmt_opt(summary.patternia_mean_ns, '.3f')} | {summary.fastest_impl} | "
            f"{summary.fastest_mean_ns:.3f} | {gap} | "
            f"{_fmt_opt(summary.patternia_cv_pct, '.2f')} | {summary.status} |"
        )

    lines += ["", "---", "", "## Per-Scenario Details"]

    for scenario in scenarios:
        row = dict(sorted(nested[scenario].items(), key=lambda kv: kv[1].mean_ns))
        fastest = min(row.values(), key=lambda x: x.mean_ns)
        pat_ref = _patternia_best(row)
        lines.append(f"### {scenario}")
        lines.append("")
        lines.append("| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |")
        lines.append("|---|---:|---:|---:|---:|")

        detail_impls = [impl for impl in row]
        detail_impls += [impl for impl in impl_order if impl in row and impl not in detail_impls]
        for impl in detail_impls:
            p = row.get(impl)
            if p is None:
                continue
            delta = (
                ((p.mean_ns - fastest.mean_ns) / fastest.mean_ns) * 100.0
                if fastest.mean_ns > 0
                else math.nan
            )
            delta_vs_pat = (
                f"{(p.mean_ns / pat_ref.mean_ns - 1.0) * 100.0:+.2f}%"
                if pat_ref and pat_ref is not p and pat_ref.mean_ns > 0
                else "-"
            ) if pat_ref else "-"
            cv = "-" if p.cv_pct is None else f"{p.cv_pct:.2f}"
            delta_s = "fastest" if p is fastest else f"{delta:+.2f}%"
            lines.append(
                f"| {'**' + impl + '**' if impl in PATTERNIA_IMPLS else impl} | "
                f"{p.mean_ns:.3f} | {delta_s} | {delta_vs_pat} | {cv} |"
            )
        lines.append("")

    out_md.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _plot(
    nested: Dict[str, Dict[str, BenchPoint]],
    scenarios: List[str],
    impl_order: List[str],
    summaries: Dict[str, ScenarioSummary],
    out_png: Path,
    title: str,
) -> None:
    try:
        import matplotlib.pyplot as plt
        from matplotlib.lines import Line2D
    except Exception as exc:  # pragma: no cover
        raise SystemExit(
            "matplotlib is required. Install: py -3 -m pip install matplotlib"
        ) from exc

    if not scenarios:
        raise SystemExit("No implementations to plot.")

    _ = impl_order

    plt.style.use("seaborn-v0_8-whitegrid")
    plt.rcParams.update(
        {
            "font.size": 10.5,
            "axes.titlesize": 13.5,
            "axes.labelsize": 11.5,
            "legend.fontsize": 10.5,
        }
    )

    max_ratio = 1.0
    for scenario in scenarios:
        fastest = summaries[scenario].fastest_mean_ns
        if fastest <= 0:
            continue
        max_ratio = max(max_ratio, *(p.mean_ns / fastest for p in nested[scenario].values()))

    x_max = max(1.35, max_ratio * 1.14)
    n = len(scenarios)
    fig_h = max(6.0, n * 0.72 + 3.2)
    fig, ax = plt.subplots(figsize=(14.2, fig_h), dpi=220)
    fig.patch.set_facecolor("#f4f1ea")
    ax.set_facecolor("#fffdf8")
    # Dynamically size the axes area to the figure height.
    ax_bottom = 0.26
    ax_height = min(0.64, max(0.40, n * 0.052))
    ax.set_position([0.14, ax_bottom, 0.80, ax_height])

    # Reading bands turn relative performance into a quick visual filter.
    ax.axvspan(1.0, min(1.05, x_max), color="#dcfce7", alpha=0.75, zorder=0)
    if x_max > 1.05:
        ax.axvspan(1.05, min(1.20, x_max), color="#fef3c7", alpha=0.72, zorder=0)
    if x_max > 1.20:
        ax.axvspan(1.20, x_max, color="#fee2e2", alpha=0.58, zorder=0)

    status_colors = {
        "fastest": "#059669",
        "close": "#2563eb",
        "watch": "#d97706",
        "slow": "#dc2626",
        "missing": "#64748b",
    }
    competitor_color = "#9aa3b2"
    line_color = "#cbd5e1"

    for yi, scenario in enumerate(scenarios):
        row = sorted(nested[scenario].values(), key=lambda p: p.mean_ns)
        summary = summaries[scenario]
        fastest = summary.fastest_mean_ns
        ratios = [p.mean_ns / fastest for p in row] if fastest > 0 else [1.0]
        row_max = max(ratios)

        ax.hlines(yi, 1.0, row_max, color=line_color, linewidth=6.0, alpha=0.92, zorder=1)
        for p, ratio in zip(row, ratios):
            if p.impl in PATTERNIA_IMPLS:
                continue
            ax.scatter(
                ratio,
                yi,
                s=42,
                color=competitor_color,
                edgecolor="#ffffff",
                linewidth=0.8,
                zorder=3,
            )

        ax.scatter(
            1.0,
            yi,
            marker="|",
            s=260,
            color="#111827",
            linewidth=2.1,
            zorder=4,
        )
        ax.text(
            1.006,
            yi + 0.20,
            f"fastest: {summary.fastest_impl} {summary.fastest_mean_ns:.3f} ns",
            fontsize=8.2,
            color="#475569",
            va="center",
        )

        if summary.patternia_ratio is None or summary.patternia_mean_ns is None:
            ax.text(
                1.02,
                yi - 0.18,
                "Patternia result missing",
                fontsize=9.0,
                color=status_colors["missing"],
                va="center",
                fontweight="bold",
            )
            continue

        pat_color = status_colors[summary.status]
        ax.scatter(
            summary.patternia_ratio,
            yi,
            marker="D",
            s=112,
            color=pat_color,
            edgecolor="#0f172a",
            linewidth=0.7,
            zorder=5,
        )
        gap = summary.patternia_delta_pct or 0.0
        ax.annotate(
            f"{summary.patternia_impl} {summary.patternia_ratio:.2f}x ({gap:+.1f}%)",
            xy=(summary.patternia_ratio, yi),
            xytext=(12, 12),
            textcoords="offset points",
            fontsize=8.8,
            color=pat_color,
            va="bottom",
            ha="left",
            fontweight="bold",
            bbox=dict(
                boxstyle="round,pad=0.22",
                facecolor="#fffdf8",
                edgecolor="none",
                alpha=0.90,
            ),
            zorder=6,
        )

    labels = [_label(s) for s in scenarios]
    label_size = 11.5 if len(scenarios) <= 6 else 10.0
    ax.set_yticks(list(range(len(scenarios))))
    ax.set_yticklabels(labels, fontsize=label_size, color="#0f172a")
    ax.invert_yaxis()
    ax.set_ylim(len(scenarios) - 0.25, -0.55)
    ax.set_xlim(0.97, x_max)
    ax.set_xlabel("Relative mean CPU time vs fastest in scenario", color="#334155", labelpad=12)
    ax.set_title("")
    ax.grid(axis="x", color="#d6d3cc", alpha=0.75)
    ax.grid(axis="y", visible=False)
    ax.spines[["top", "right", "left"]].set_visible(False)
    ax.spines["bottom"].set_color("#94a3b8")
    ax.tick_params(axis="y", length=0)

    wins = sum(1 for s in scenarios if summaries[s].status == "fastest")
    covered = [summaries[s] for s in scenarios if summaries[s].patternia_delta_pct is not None]
    avg_gap = sum(s.patternia_delta_pct for s in covered if s.patternia_delta_pct is not None) / max(1, len(covered))
    worst = max(
        covered,
        key=lambda s: s.patternia_delta_pct if s.patternia_delta_pct is not None else -1.0,
        default=None,
    )
    worst_label = "n/a" if worst is None else f"{_label(worst.scenario)} {worst.patternia_delta_pct:+.1f}%"
    summary_line = (
        f"{len(scenarios)} scenarios    |    "
        f"Patternia fastest {wins}/{len(scenarios)}    |    "
        f"avg gap {avg_gap:+.1f}%    |    "
        f"largest gap {worst_label}"
    )
    fig.text(
        0.5,
        0.855,
        summary_line,
        ha="center",
        va="center",
        fontsize=10.8,
        color="#0f172a",
        bbox=dict(
            boxstyle="round,pad=0.55",
            facecolor="#fffdf8",
            edgecolor="#d6d3d1",
            linewidth=1.0,
        ),
    )

    legend = [
        Line2D([0], [0], marker="D", color="w", label="Patternia fastest", markerfacecolor=status_colors["fastest"], markeredgecolor="#0f172a", markersize=8),
        Line2D([0], [0], marker="D", color="w", label="Patternia <= 5% behind", markerfacecolor=status_colors["close"], markeredgecolor="#0f172a", markersize=8),
        Line2D([0], [0], marker="D", color="w", label="Patternia 5-20% behind", markerfacecolor=status_colors["watch"], markeredgecolor="#0f172a", markersize=8),
        Line2D([0], [0], marker="D", color="w", label="Patternia > 20% behind", markerfacecolor=status_colors["slow"], markeredgecolor="#0f172a", markersize=8),
        Line2D([0], [0], marker="o", color="w", label="Other implementation", markerfacecolor=competitor_color, markeredgecolor="#ffffff", markersize=7),
        Line2D([0], [0], marker="|", color="#111827", label="Fastest implementation", markersize=13, linestyle="None"),
    ]
    fig.legend(
        handles=legend,
        loc="lower center",
        bbox_to_anchor=(0.5, 0.075),
        ncol=3,
        frameon=False,
        fontsize=9.7,
        columnspacing=1.6,
        handletextpad=0.6,
    )

    fig.suptitle(title, fontsize=22, fontweight="bold", color="#0f172a", y=0.958)
    fig.text(
        0.5,
        0.785,
        "Patternia gap map - lower and further left is better",
        ha="center",
        va="center",
        fontsize=13.5,
        fontweight="bold",
        color="#0f172a",
    )
    fig.text(
        0.5,
        0.022,
        "Each row is normalized to its own fastest implementation. Patternia is the diamond; grey dots are alternatives.",
        ha="center",
        va="bottom",
        fontsize=8.0,
        color="#64748b",
    )

    fig.savefig(out_png)
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Visualize one benchmark JSON across implementations."
    )
    parser.add_argument("--input", default=str(DEFAULT_INPUT), help="Input benchmark JSON or generated CSV")
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

    include_regex = args.include.strip() or None
    if input_path.suffix.lower() == ".csv":
        points = _points_from_csv(input_path, include_regex)
    else:
        payload = _load_json(input_path)
        metrics = _extract_metrics(payload)
        points = _to_points(metrics, include_regex)
    if not points:
        raise SystemExit("No benchmark entries found after filtering.")

    nested = _to_nested(points)
    raw_scenarios = sorted(nested.keys())
    summaries = _summaries(nested, raw_scenarios)
    scenarios = _scenario_order(summaries)
    impl_order = _default_impl_order(p.impl for p in points)

    out_md = outdir / f"{args.prefix}.md"
    out_csv = outdir / f"{args.prefix}.csv"
    out_png = outdir / f"{args.prefix}.png"

    _save_markdown(nested, scenarios, impl_order, summaries, input_path, out_md)
    _save_csv(points, summaries, out_csv)
    if not args.no_plot:
        _plot(nested, scenarios, impl_order, summaries, out_png, args.title)

    print(f"Saved markdown: {out_md}")
    print(f"Saved csv: {out_csv}")
    if not args.no_plot:
        print(f"Saved chart: {out_png}")


if __name__ == "__main__":
    main()
