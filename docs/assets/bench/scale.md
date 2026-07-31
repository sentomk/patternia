# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+92.36%`
- Largest Patternia gap: `ScaleN32` `+136.63%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.136 | SwitchIndex | 0.903 | +136.63% | 0.42 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.567 | StdVisit | 0.821 | +90.83% | 1.14 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.765 | SwitchIndex | 0.928 | +90.31% | 1.72 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.360 | SwitchIndex | 0.897 | +51.66% | 0.86 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.903 | fastest | -57.74% | 0.40 |
| **PatterniaPipe** | 2.136 | +136.63% | - | 0.42 |
| StdVisit | 8.200 | +808.40% | +283.89% | 2.88 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.821 | fastest | -47.60% | 0.20 |
| SwitchIndex | 0.831 | +1.21% | -46.96% | 4.84 |
| **PatterniaPipe** | 1.567 | +90.83% | - | 1.14 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.928 | fastest | -47.45% | 2.38 |
| **PatterniaPipe** | 1.765 | +90.31% | - | 1.72 |
| StdVisit | 5.866 | +532.40% | +232.30% | 7.72 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.897 | fastest | -34.06% | 0.69 |
| StdVisit | 0.935 | +4.21% | -31.29% | 0.30 |
| **PatterniaPipe** | 1.360 | +51.66% | - | 0.86 |

