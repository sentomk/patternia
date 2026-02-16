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

## Optional args

```powershell
py -3 scripts/bench_compare.py \
  --include "PacketMixedHeavyBind" \
  --label-baseline "backup" \
  --label-current "current" \
  --prefix "packet_heavy"
```

## Dependency

```powershell
py -3 -m pip install matplotlib
```
