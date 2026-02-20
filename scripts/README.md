# Benchmark Scripts

## 1) Stage two result files into a standard location

```powershell
py -3 scripts/bench_stage_results.py \
  --baseline "F:/code-backup/patternia/build/bench/result.json" \
  --current "F:/code/patternia/build/bench/result.json"
```

This writes:

- `bench/results/baseline/result.json`
- `bench/results/current/result.json`

(If you pass a directory, the script auto-appends `result.json`.)

## 2) Generate chart + markdown report

```powershell
py -3 scripts/bench_compare.py
```

Default outputs:

- `build/bench/compare/bench_compare.png`
- `build/bench/compare/bench_compare.md`

## 2.1) Single-file visualization (multi-impl in one JSON)

Use this when one JSON already contains multiple implementations
(Patternia / if-else / switch / std::visit) and you want per-scenario
comparison.

```powershell
py -3 scripts/bench_single_report.py `
  --input build/variant_all.json `
  --include "Variant|ProtocolRouter|CommandParser" `
  --outdir build/bench/single `
  --prefix single_impl
```

Outputs:

- `build/bench/single/single_impl.png`
- `build/bench/single/single_impl.md`
- `build/bench/single/single_impl.csv`

## Optional args

```powershell
py -3 scripts/bench_compare.py \
  --include "PacketMixedHeavyBind" \
  --label-baseline "backup" \
  --label-current "current" \
  --prefix "packet_heavy"
```

## 3) Stable variant run profile (mixed + microbench)

Variant benchmarks now include an in-code stable profile (ns unit, min time,
repetitions, aggregate report). You can run the variant suite directly:

```powershell
.\build\bench\ptn_bench.exe `
  --benchmark_filter="Variant(Mixed|AltHot)" `
  --benchmark_out=build/bench/result.json `
  --benchmark_out_format=json
```

## 4) Regression gate (CI-friendly)

```powershell
py -3 scripts/bench_compare.py \
  --include "Variant(Mixed|AltHot)" \
  --fail-if-regress-pct 3.0 \
  --fail-if-mean-regress-pct 1.5
```

- `--fail-if-regress-pct`: fail when any selected benchmark regresses beyond threshold.
- `--fail-if-mean-regress-pct`: fail when mean regression across selected benchmarks exceeds threshold.

## Dependency

```powershell
py -3 -m pip install matplotlib
```
