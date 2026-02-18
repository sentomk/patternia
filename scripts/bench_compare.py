#!/usr/bin/env python3
"""Compare and visualize two Google Benchmark JSON files.

Default input paths (repo-relative):
- bench/results/baseline/result.json
- bench/results/current/result.json
"""

from __future__ import annotations

import argparse
import json
import math
import re
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BASELINE = REPO_ROOT / "bench" / "results" / "baseline" / "result.json"
DEFAULT_CURRENT = REPO_ROOT / "bench" / "results" / "current" / "result.json"
DEFAULT_OUTDIR = REPO_ROOT / "build" / "bench" / "compare"


@dataclass
class BenchRow:
    name: str
    baseline_mean_ns: float
    current_mean_ns: float
    delta_pct: float
    speedup: float
    baseline_cv_pct: Optional[float]
    current_cv_pct: Optional[float]


@dataclass
class ContextInfo:
    date: str
    host_name: str
    executable: str
    library_build_type: str


def _load_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as f:
        return json.load(f)


def _extract_context(payload: dict) -> ContextInfo:
    c = payload.get("context", {})
    return ContextInfo(
        date=str(c.get("date", "")),
        host_name=str(c.get("host_name", "")),
        executable=str(c.get("executable", "")),
        library_build_type=str(c.get("library_build_type", "")),
    )


def _extract_aggregates(payload: dict) -> Dict[str, Dict[str, float]]:
    out: Dict[str, Dict[str, float]] = {}
    for entry in payload.get("benchmarks", []):
        run_name = entry.get("run_name")
        agg = entry.get("aggregate_name")
        if not run_name or not agg:
            continue

        slot = out.setdefault(str(run_name), {})
        cpu_time = entry.get("cpu_time")
        if cpu_time is not None:
            slot[str(agg)] = float(cpu_time)
    return out


def _to_rows(
    baseline: Dict[str, Dict[str, float]],
    current: Dict[str, Dict[str, float]],
    include_regex: Optional[str],
) -> List[BenchRow]:
    names = sorted(set(baseline) & set(current))
    if include_regex:
        pat = re.compile(include_regex)
        names = [n for n in names if pat.search(n)]

    rows: List[BenchRow] = []
    for name in names:
        b_mean = baseline[name].get("mean")
        c_mean = current[name].get("mean")
        if b_mean is None or c_mean is None:
            continue

        delta_pct = ((c_mean - b_mean) / b_mean * 100.0) if b_mean != 0 else math.nan
        speedup = (b_mean / c_mean) if c_mean != 0 else math.nan

        b_cv = baseline[name].get("cv")
        c_cv = current[name].get("cv")

        rows.append(
            BenchRow(
                name=name,
                baseline_mean_ns=b_mean,
                current_mean_ns=c_mean,
                delta_pct=delta_pct,
                speedup=speedup,
                baseline_cv_pct=(b_cv * 100.0 if b_cv is not None else None),
                current_cv_pct=(c_cv * 100.0 if c_cv is not None else None),
            )
        )
    return rows


def _is_finite(x: float) -> bool:
    return not math.isnan(x) and not math.isinf(x)


def _find_regressions(rows: List[BenchRow], max_regress_pct: float) -> List[BenchRow]:
    return [r for r in rows if _is_finite(r.delta_pct) and r.delta_pct > max_regress_pct]


def _display_name(name: str, max_len: int) -> Tuple[str, bool]:
    cleaned = name[3:] if name.startswith("BM_") else name
    cleaned = cleaned.replace("_", " ")
    if len(cleaned) <= max_len:
        return cleaned, False
    return cleaned[: max_len - 3] + "...", True


def _display_names(rows: List[BenchRow], max_len: int) -> Tuple[List[str], List[Tuple[str, str]]]:
    display: List[str] = []
    truncated: List[Tuple[str, str]] = []
    for r in rows:
        short, is_trunc = _display_name(r.name, max_len)
        display.append(short)
        if is_trunc:
            truncated.append((short, r.name))
    return display, truncated


def _save_markdown(
    rows: List[BenchRow],
    out_md: Path,
    baseline_label: str,
    current_label: str,
    baseline_path: Path,
    current_path: Path,
    baseline_ctx: ContextInfo,
    current_ctx: ContextInfo,
) -> None:
    lines = [
        "# Benchmark Comparison Report",
        "",
        f"- Baseline label: `{baseline_label}`",
        f"- Current label: `{current_label}`",
        f"- Baseline file: `{baseline_path}`",
        f"- Current file: `{current_path}`",
        "",
        "## Context",
        "",
        f"- Baseline date: `{baseline_ctx.date}`",
        f"- Current date: `{current_ctx.date}`",
        f"- Host: `{current_ctx.host_name or baseline_ctx.host_name}`",
        f"- Build type: baseline=`{baseline_ctx.library_build_type}`, current=`{current_ctx.library_build_type}`",
        "",
        "## Results",
        "",
        "| Benchmark | Baseline mean (ns) | Current mean (ns) | Delta % | Speedup (x) | Baseline CV % | Current CV % |",
        "|---|---:|---:|---:|---:|---:|---:|",
    ]

    for r in rows:
        bcv = f"{r.baseline_cv_pct:.2f}" if r.baseline_cv_pct is not None else "-"
        ccv = f"{r.current_cv_pct:.2f}" if r.current_cv_pct is not None else "-"
        lines.append(
            f"| {r.name} | {r.baseline_mean_ns:.3f} | {r.current_mean_ns:.3f} | "
            f"{r.delta_pct:+.2f}% | {r.speedup:.3f} | {bcv} | {ccv} |"
        )

    out_md.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _plot(
    rows: List[BenchRow],
    out_png: Path,
    baseline_label: str,
    current_label: str,
    baseline_ctx: ContextInfo,
    current_ctx: ContextInfo,
    name_max_len: int,
) -> None:
    try:
        import matplotlib.pyplot as plt
        from matplotlib import patheffects
    except Exception as exc:  # pragma: no cover
        raise SystemExit("matplotlib is required. Install: py -3 -m pip install matplotlib") from exc

    plt.style.use("seaborn-v0_8-whitegrid")

    names = [r.name for r in rows]
    display_names, trunc_map = _display_names(rows, name_max_len)

    bvals = [r.baseline_mean_ns for r in rows]
    cvals = [r.current_mean_ns for r in rows]
    deltas = [r.delta_pct for r in rows]

    baseline_color = "#64748b"
    current_color = "#0ea5e9"
    better_color = "#16a34a"
    worse_color = "#dc2626"

    n = max(1, len(rows))
    fig_w = max(12.0, 1.8 * n + 6.0)
    fig_h = 9.0 if n > 1 else 7.8

    fig = plt.figure(figsize=(fig_w, fig_h), dpi=220, constrained_layout=False)
    fig.patch.set_facecolor("#f8fafc")
    gs = fig.add_gridspec(
        2,
        1,
        height_ratios=[1.45, 1.0],
        left=0.06,
        right=0.98,
        bottom=0.09,
        top=0.80,
        hspace=0.40,
    )

    ax1 = fig.add_subplot(gs[0, 0])
    ax2 = fig.add_subplot(gs[1, 0])

    for ax in (ax1, ax2):
        ax.set_facecolor("#ffffff")

    idx = list(range(len(rows)))
    width = 0.38

    bars_b = ax1.bar(
        [i - width / 2 for i in idx],
        bvals,
        width=width,
        color=baseline_color,
        label=baseline_label,
        edgecolor="#334155",
        linewidth=0.4,
    )
    bars_c = ax1.bar(
        [i + width / 2 for i in idx],
        cvals,
        width=width,
        color=current_color,
        label=current_label,
        edgecolor="#0c4a6e",
        linewidth=0.4,
    )

    ax1.set_title(
        "CPU Mean Time (lower is better)",
        fontsize=14,
        fontweight="bold",
        color="#0f172a",
        pad=10,
    )
    ax1.set_ylabel("ns", fontsize=11)
    ax1.set_xticks(idx)

    if n == 1:
        ax1.set_xticklabels(display_names, rotation=0, ha="center", fontsize=11)
    else:
        ax1.set_xticklabels(display_names, rotation=14, ha="right", fontsize=10)

    ax1.legend(frameon=False, fontsize=11)

    ymax = max(max(bvals, default=0.0), max(cvals, default=0.0))
    ax1.set_ylim(0.0, ymax * 1.25 if ymax > 0 else 1.0)

    for bars in (bars_b, bars_c):
        for bar in bars:
            h = bar.get_height()
            ax1.text(
                bar.get_x() + bar.get_width() / 2,
                h + (ymax * 0.02 if ymax > 0 else 0.02),
                f"{h:.2f}",
                ha="center",
                va="bottom",
                fontsize=9,
                color="#0f172a",
            )

    if n == 1:
        row = rows[0]
        delta_color = better_color if row.delta_pct < 0 else worse_color

        ax2.axis("off")
        ax2.set_title("Summary", fontsize=14, fontweight="bold", color="#0f172a", pad=12)

        box_style = dict(boxstyle="round,pad=0.55", facecolor="#ffffff", edgecolor="#cbd5e1", linewidth=1.1)

        ax2.text(0.15, 0.68, "Delta vs Baseline", fontsize=11, color="#334155", transform=ax2.transAxes)
        ax2.text(
            0.15,
            0.46,
            f"{row.delta_pct:+.2f}%",
            fontsize=23,
            fontweight="bold",
            color=delta_color,
            transform=ax2.transAxes,
            bbox=box_style,
        )

        ax2.text(0.55, 0.68, "Speedup", fontsize=11, color="#334155", transform=ax2.transAxes)
        ax2.text(
            0.55,
            0.46,
            f"{row.speedup:.2f}x",
            fontsize=23,
            fontweight="bold",
            color="#0f172a",
            transform=ax2.transAxes,
            bbox=box_style,
        )

        bcv = f"{row.baseline_cv_pct:.2f}%" if row.baseline_cv_pct is not None else "-"
        ccv = f"{row.current_cv_pct:.2f}%" if row.current_cv_pct is not None else "-"
        ax2.text(
            0.15,
            0.18,
            f"CV baseline: {bcv}   CV current: {ccv}",
            fontsize=10,
            color="#475569",
            transform=ax2.transAxes,
        )
    else:
        colors = [better_color if d < 0 else worse_color for d in deltas]
        bars = ax2.barh(display_names, deltas, color=colors, alpha=0.95)
        ax2.axvline(0.0, color="#111827", linewidth=1.1)
        ax2.set_title("Delta vs Baseline (%)  (negative is faster)", fontsize=14, fontweight="bold", color="#0f172a")
        ax2.set_xlabel("%", fontsize=11)

        max_abs = max((abs(d) for d in deltas), default=1.0)
        lim = max(5.0, max_abs * 1.2)
        ax2.set_xlim(-lim, lim)

        for bar, delta in zip(bars, deltas):
            x = bar.get_width()
            y = bar.get_y() + bar.get_height() / 2
            txt = f"{delta:+.2f}%"
            ha = "left" if x >= 0 else "right"
            pad = 0.4 if x >= 0 else -0.4
            ax2.text(
                x + pad,
                y,
                txt,
                va="center",
                ha=ha,
                fontsize=10,
                color="#0f172a",
                path_effects=[patheffects.withStroke(linewidth=2.2, foreground="white")],
            )

    avg_delta = sum(deltas) / len(deltas) if deltas else 0.0
    mean_speedup = sum((r.speedup for r in rows if not math.isnan(r.speedup))) / max(1, len(rows))

    subtitle = (
        f"Baseline: {baseline_label}   Current: {current_label}   "
        f"Avg delta: {avg_delta:+.2f}%   Mean speedup: {mean_speedup:.2f}x"
    )

    fig.suptitle(
        "Patternia Benchmark Comparison",
        fontsize=21,
        fontweight="bold",
        color="#0f172a",
        y=0.985,
    )
    fig.text(0.5, 0.948, subtitle, ha="center", va="top", fontsize=10, color="#334155")

    ctx_line = (
        f"Baseline date: {baseline_ctx.date} | Current date: {current_ctx.date} | "
        f"Host: {current_ctx.host_name or baseline_ctx.host_name}"
    )
    fig.text(0.5, 0.02, ctx_line, ha="center", va="bottom", fontsize=9, color="#475569")

    if trunc_map:
        pairs = [f"{short} => {full}" for short, full in trunc_map]
        fig.text(0.5, 0.045, "; ".join(pairs), ha="center", va="bottom", fontsize=8.5, color="#64748b")

    fig.savefig(out_png, dpi=220)
    plt.close(fig)


def main() -> int:
    parser = argparse.ArgumentParser(description="Compare and visualize benchmark JSON files.")
    parser.add_argument("--baseline", default=str(DEFAULT_BASELINE), help="Baseline result.json path")
    parser.add_argument("--current", default=str(DEFAULT_CURRENT), help="Current result.json path")
    parser.add_argument("--label-baseline", default="baseline", help="Legend label for baseline")
    parser.add_argument("--label-current", default="current", help="Legend label for current")
    parser.add_argument("--include", default="", help="Regex: only include matching benchmark names")
    parser.add_argument("--name-max-len", type=int, default=28, help="Max display length for benchmark names")
    parser.add_argument("--outdir", default=str(DEFAULT_OUTDIR), help="Output directory")
    parser.add_argument("--prefix", default="bench_compare", help="Output file prefix")
    parser.add_argument(
        "--fail-if-regress-pct",
        type=float,
        default=None,
        help="Fail if any benchmark delta %% is greater than this threshold (higher is slower).",
    )
    parser.add_argument(
        "--fail-if-mean-regress-pct",
        type=float,
        default=None,
        help="Fail if mean delta %% across selected rows is greater than this threshold.",
    )
    args = parser.parse_args()

    baseline_path = Path(args.baseline).expanduser().resolve()
    current_path = Path(args.current).expanduser().resolve()
    outdir = Path(args.outdir).expanduser().resolve()
    outdir.mkdir(parents=True, exist_ok=True)

    if not baseline_path.exists():
        raise SystemExit(f"baseline file not found: {baseline_path}")
    if not current_path.exists():
        raise SystemExit(f"current file not found: {current_path}")

    baseline_payload = _load_json(baseline_path)
    current_payload = _load_json(current_path)

    baseline_ctx = _extract_context(baseline_payload)
    current_ctx = _extract_context(current_payload)

    baseline_agg = _extract_aggregates(baseline_payload)
    current_agg = _extract_aggregates(current_payload)

    rows = _to_rows(baseline_agg, current_agg, args.include.strip() or None)
    if not rows:
        raise SystemExit("No overlapping benchmark aggregates found (check inputs / --include).")

    out_png = outdir / f"{args.prefix}.png"
    out_md = outdir / f"{args.prefix}.md"

    _plot(
        rows,
        out_png,
        args.label_baseline,
        args.label_current,
        baseline_ctx,
        current_ctx,
        max(10, args.name_max_len),
    )
    _save_markdown(
        rows,
        out_md,
        args.label_baseline,
        args.label_current,
        baseline_path,
        current_path,
        baseline_ctx,
        current_ctx,
    )

    print(f"Saved chart: {out_png}")
    print(f"Saved report: {out_md}")

    exit_code = 0
    if args.fail_if_regress_pct is not None:
        regressions = _find_regressions(rows, args.fail_if_regress_pct)
        if regressions:
            print(
                f"[bench-gate] FAIL: {len(regressions)} benchmark(s) exceed "
                f"+{args.fail_if_regress_pct:.2f}% regression threshold."
            )
            for r in regressions:
                print(f"  - {r.name}: {r.delta_pct:+.2f}%")
            exit_code = 2

    if args.fail_if_mean_regress_pct is not None:
        deltas = [r.delta_pct for r in rows if _is_finite(r.delta_pct)]
        if deltas:
            mean_delta = sum(deltas) / len(deltas)
            if mean_delta > args.fail_if_mean_regress_pct:
                print(
                    "[bench-gate] FAIL: mean delta "
                    f"{mean_delta:+.2f}% exceeds +{args.fail_if_mean_regress_pct:.2f}%."
                )
                exit_code = 2

    if exit_code == 0 and (
        args.fail_if_regress_pct is not None or args.fail_if_mean_regress_pct is not None
    ):
        print("[bench-gate] PASS: regression thresholds satisfied.")

    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
