# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+79.85%`
- Largest Patternia gap: `ScaleN32` `+123.86%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.260 | SwitchIndex | 1.010 | +123.86% | 0.64 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.742 | SwitchIndex | 0.919 | +89.55% | 1.69 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.705 | SwitchIndex | 1.015 | +67.90% | 1.25 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.398 | SwitchIndex | 1.012 | +38.08% | 0.54 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.010 | fastest | -55.33% | 0.61 |
| **PatterniaPipe** | 2.260 | +123.86% | - | 0.64 |
| StdVisit | 6.582 | +552.03% | +191.26% | 0.13 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.919 | fastest | -47.24% | 0.20 |
| StdVisit | 0.954 | +3.74% | -45.27% | 0.56 |
| **PatterniaPipe** | 1.742 | +89.55% | - | 1.69 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.015 | fastest | -40.44% | 0.81 |
| **PatterniaPipe** | 1.705 | +67.90% | - | 1.25 |
| StdVisit | 3.121 | +207.37% | +83.07% | 0.82 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.012 | fastest | -27.58% | 0.18 |
| StdVisit | 1.053 | +4.01% | -24.67% | 0.22 |
| **PatterniaPipe** | 1.398 | +38.08% | - | 0.54 |

