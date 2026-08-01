# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+89.76%`
- Largest Patternia gap: `ScaleN32` `+137.00%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.139 | SwitchIndex | 0.902 | +137.00% | 0.71 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.566 | StdVisit | 0.824 | +90.15% | 0.90 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.664 | SwitchIndex | 0.921 | +80.59% | 0.27 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.356 | SwitchIndex | 0.896 | +51.32% | 1.35 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.902 | fastest | -57.81% | 0.19 |
| **PatterniaPipe** | 2.139 | +137.00% | - | 0.71 |
| StdVisit | 8.204 | +809.11% | +283.60% | 1.40 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.824 | fastest | -47.41% | 0.22 |
| SwitchIndex | 0.835 | +1.44% | -46.65% | 4.86 |
| **PatterniaPipe** | 1.566 | +90.15% | - | 0.90 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.921 | fastest | -44.63% | 1.21 |
| **PatterniaPipe** | 1.664 | +80.59% | - | 0.27 |
| StdVisit | 5.741 | +523.05% | +245.01% | 2.86 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.896 | fastest | -33.91% | 0.22 |
| StdVisit | 0.935 | +4.37% | -31.03% | 0.27 |
| **PatterniaPipe** | 1.356 | +51.32% | - | 1.35 |

