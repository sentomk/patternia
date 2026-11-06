# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+92.00%`
- Largest Patternia gap: `ScaleN32` `+136.89%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.137 | SwitchIndex | 0.902 | +136.89% | 0.46 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.795 | SwitchIndex | 0.924 | +94.12% | 6.64 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.553 | StdVisit | 0.825 | +88.18% | 1.60 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.345 | SwitchIndex | 0.904 | +48.80% | 0.73 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.902 | fastest | -57.79% | 0.11 |
| **PatterniaPipe** | 2.137 | +136.89% | - | 0.46 |
| StdVisit | 6.950 | +670.41% | +225.22% | 7.54 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.924 | fastest | -48.49% | 2.30 |
| **PatterniaPipe** | 1.795 | +94.12% | - | 6.64 |
| StdVisit | 5.537 | +498.98% | +208.56% | 0.34 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.825 | fastest | -46.86% | 0.22 |
| SwitchIndex | 0.902 | +9.34% | -41.90% | 3.59 |
| **PatterniaPipe** | 1.553 | +88.18% | - | 1.60 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.904 | fastest | -32.80% | 0.77 |
| StdVisit | 0.935 | +3.45% | -30.48% | 0.16 |
| **PatterniaPipe** | 1.345 | +48.80% | - | 0.73 |

