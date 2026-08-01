# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+90.42%`
- Largest Patternia gap: `ScaleN32` `+145.60%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.220 | SwitchIndex | 0.904 | +145.60% | 1.03 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.561 | StdVisit | 0.830 | +88.05% | 0.98 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.660 | SwitchIndex | 0.937 | +77.14% | 0.41 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.350 | SwitchIndex | 0.895 | +50.86% | 2.48 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.904 | fastest | -59.28% | 0.34 |
| **PatterniaPipe** | 2.220 | +145.60% | - | 1.03 |
| StdVisit | 7.676 | +749.36% | +245.82% | 8.16 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.830 | fastest | -46.82% | 0.26 |
| SwitchIndex | 0.873 | +5.22% | -44.05% | 4.65 |
| **PatterniaPipe** | 1.561 | +88.05% | - | 0.98 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.937 | fastest | -43.55% | 1.97 |
| **PatterniaPipe** | 1.660 | +77.14% | - | 0.41 |
| StdVisit | 5.683 | +506.32% | +242.28% | 2.53 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.895 | fastest | -33.72% | 0.13 |
| StdVisit | 0.935 | +4.48% | -30.75% | 0.12 |
| **PatterniaPipe** | 1.350 | +50.86% | - | 2.48 |

