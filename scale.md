# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `1/4`
- Average Patternia gap vs fastest: `+7.15%`
- Largest Patternia gap: `ScaleN4` `+15.02%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN4 | PatterniaPipe | 3/3 | 0.648 | SwitchIndex | 0.563 | +15.02% | 0.09 | watch |
| ScaleN8 | PatterniaPipe | 2/3 | 0.769 | StdVisit | 0.689 | +11.50% | 0.10 | watch |
| ScaleN32 | PatterniaPipe | 2/3 | 0.680 | SwitchIndex | 0.666 | +2.10% | 3.03 | close |
| ScaleN16 | PatterniaPipe | 1/3 | 0.549 | PatterniaPipe | 0.549 | +0.00% | 0.29 | fastest |

---

## Per-Scenario Details
### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.563 | fastest | -13.06% | 0.11 |
| StdVisit | 0.644 | +14.32% | -0.61% | 0.36 |
| **PatterniaPipe** | 0.648 | +15.02% | - | 0.09 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.689 | fastest | -10.31% | 0.13 |
| **PatterniaPipe** | 0.769 | +11.50% | - | 0.10 |
| SwitchIndex | 0.770 | +11.62% | +0.11% | 0.31 |

### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.666 | fastest | -2.05% | 1.66 |
| **PatterniaPipe** | 0.680 | +2.10% | - | 3.03 |
| StdVisit | 1.573 | +136.17% | +131.32% | 0.11 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 0.549 | fastest | - | 0.29 |
| SwitchIndex | 0.553 | +0.73% | +0.73% | 0.22 |
| StdVisit | 1.585 | +188.55% | +188.55% | 0.08 |

