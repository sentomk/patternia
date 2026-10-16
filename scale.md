# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+79.91%`
- Largest Patternia gap: `ScaleN32` `+120.16%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 1.721 | SwitchIndex | 0.782 | +120.16% | 1.75 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.395 | SwitchIndex | 0.726 | +92.17% | 3.46 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.331 | SwitchIndex | 0.787 | +69.16% | 1.30 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.084 | SwitchIndex | 0.785 | +38.14% | 0.64 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.782 | fastest | -54.58% | 0.08 |
| **PatterniaPipe** | 1.721 | +120.16% | - | 1.75 |
| StdVisit | 5.189 | +563.65% | +201.44% | 1.54 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.726 | fastest | -47.96% | 3.49 |
| StdVisit | 0.738 | +1.60% | -47.13% | 0.17 |
| **PatterniaPipe** | 1.395 | +92.17% | - | 3.46 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.787 | fastest | -40.89% | 0.53 |
| **PatterniaPipe** | 1.331 | +69.16% | - | 1.30 |
| StdVisit | 2.374 | +201.60% | +78.29% | 1.25 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.785 | fastest | -27.61% | 0.10 |
| StdVisit | 0.817 | +4.09% | -24.65% | 0.22 |
| **PatterniaPipe** | 1.084 | +38.14% | - | 0.64 |

