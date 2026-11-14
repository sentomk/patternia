# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+47.47%`
- Largest Patternia gap: `ScaleN8` `+74.73%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN8 | PatterniaPipe | 3/3 | 0.986 | SwitchIndex | 0.565 | +74.73% | 3.14 | slow |
| ScaleN32 | PatterniaPipe | 2/3 | 0.985 | SwitchIndex | 0.586 | +68.22% | 3.58 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 0.765 | StdVisit | 0.610 | +25.37% | 2.63 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 0.812 | SwitchIndex | 0.668 | +21.54% | 3.56 | slow |

---

## Per-Scenario Details
### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.565 | fastest | -42.77% | 1.79 |
| StdVisit | 0.886 | +56.85% | -10.23% | 1.34 |
| **PatterniaPipe** | 0.986 | +74.73% | - | 3.14 |

### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.586 | fastest | -40.56% | 1.86 |
| **PatterniaPipe** | 0.985 | +68.22% | - | 3.58 |
| StdVisit | 1.668 | +184.82% | +69.31% | 2.20 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.610 | fastest | -20.24% | 2.60 |
| SwitchIndex | 0.644 | +5.63% | -15.75% | 2.08 |
| **PatterniaPipe** | 0.765 | +25.37% | - | 2.63 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.668 | fastest | -17.72% | 2.03 |
| **PatterniaPipe** | 0.812 | +21.54% | - | 3.56 |
| StdVisit | 1.689 | +152.92% | +108.10% | 3.42 |

