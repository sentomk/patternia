# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+76.95%`
- Largest Patternia gap: `ScaleN32` `+114.60%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.164 | SwitchIndex | 1.008 | +114.60% | 0.58 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.728 | SwitchIndex | 0.922 | +87.32% | 0.99 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.701 | SwitchIndex | 1.013 | +67.89% | 0.70 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.397 | SwitchIndex | 1.013 | +37.97% | 0.95 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.008 | fastest | -53.40% | 0.15 |
| **PatterniaPipe** | 2.164 | +114.60% | - | 0.58 |
| StdVisit | 6.586 | +553.22% | +204.39% | 0.21 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.922 | fastest | -46.62% | 0.82 |
| StdVisit | 0.953 | +3.34% | -44.83% | 0.59 |
| **PatterniaPipe** | 1.728 | +87.32% | - | 0.99 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.013 | fastest | -40.44% | 0.23 |
| **PatterniaPipe** | 1.701 | +67.89% | - | 0.70 |
| StdVisit | 3.132 | +209.00% | +84.05% | 0.35 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.013 | fastest | -27.52% | 0.24 |
| StdVisit | 1.055 | +4.20% | -24.47% | 0.61 |
| **PatterniaPipe** | 1.397 | +37.97% | - | 0.95 |

