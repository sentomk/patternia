# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+89.62%`
- Largest Patternia gap: `ScaleN32` `+137.13%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.140 | SwitchIndex | 0.902 | +137.13% | 1.30 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.539 | SwitchIndex | 0.808 | +90.55% | 0.17 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.660 | SwitchIndex | 0.921 | +80.21% | 0.22 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.347 | SwitchIndex | 0.895 | +50.58% | 1.03 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.902 | fastest | -57.83% | 0.19 |
| **PatterniaPipe** | 2.140 | +137.13% | - | 1.30 |
| StdVisit | 7.516 | +732.90% | +251.25% | 11.60 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.808 | fastest | -47.52% | 2.11 |
| StdVisit | 0.827 | +2.44% | -46.24% | 0.24 |
| **PatterniaPipe** | 1.539 | +90.55% | - | 0.17 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.921 | fastest | -44.51% | 1.85 |
| **PatterniaPipe** | 1.660 | +80.21% | - | 0.22 |
| StdVisit | 6.244 | +577.65% | +276.03% | 0.97 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.895 | fastest | -33.59% | 0.23 |
| StdVisit | 0.933 | +4.34% | -30.71% | 0.14 |
| **PatterniaPipe** | 1.347 | +50.58% | - | 1.03 |

