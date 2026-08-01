# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+89.17%`
- Largest Patternia gap: `ScaleN32` `+138.70%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.154 | SwitchIndex | 0.902 | +138.70% | 2.00 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.543 | StdVisit | 0.823 | +87.58% | 0.15 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.689 | SwitchIndex | 0.937 | +80.26% | 2.68 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.344 | SwitchIndex | 0.895 | +50.15% | 0.71 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.902 | fastest | -58.11% | 0.19 |
| **PatterniaPipe** | 2.154 | +138.70% | - | 2.00 |
| StdVisit | 8.488 | +840.62% | +294.06% | 3.97 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.823 | fastest | -46.69% | 0.19 |
| SwitchIndex | 0.886 | +7.72% | -42.57% | 4.49 |
| **PatterniaPipe** | 1.543 | +87.58% | - | 0.15 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.937 | fastest | -44.52% | 1.68 |
| **PatterniaPipe** | 1.689 | +80.26% | - | 2.68 |
| StdVisit | 2.897 | +209.11% | +71.48% | 20.19 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.895 | fastest | -33.40% | 0.12 |
| StdVisit | 0.935 | +4.45% | -30.44% | 0.11 |
| **PatterniaPipe** | 1.344 | +50.15% | - | 0.71 |

