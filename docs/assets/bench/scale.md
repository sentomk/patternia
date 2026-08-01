# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+76.54%`
- Largest Patternia gap: `ScaleN32` `+119.34%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.212 | SwitchIndex | 1.008 | +119.34% | 2.14 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.759 | SwitchIndex | 0.952 | +84.85% | 0.84 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.664 | SwitchIndex | 1.013 | +64.27% | 1.11 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.394 | SwitchIndex | 1.013 | +37.70% | 0.38 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.008 | fastest | -54.41% | 0.24 |
| **PatterniaPipe** | 2.212 | +119.34% | - | 2.14 |
| StdVisit | 6.600 | +554.42% | +198.36% | 0.51 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.952 | fastest | -45.90% | 4.17 |
| StdVisit | 0.952 | +0.03% | -45.89% | 0.12 |
| **PatterniaPipe** | 1.759 | +84.85% | - | 0.84 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.013 | fastest | -39.12% | 0.18 |
| **PatterniaPipe** | 1.664 | +64.27% | - | 1.11 |
| StdVisit | 3.136 | +209.65% | +88.50% | 0.92 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.013 | fastest | -27.38% | 0.15 |
| StdVisit | 1.053 | +4.04% | -24.44% | 0.32 |
| **PatterniaPipe** | 1.394 | +37.70% | - | 0.38 |

