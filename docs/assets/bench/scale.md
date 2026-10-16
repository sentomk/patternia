# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+87.69%`
- Largest Patternia gap: `ScaleN32` `+137.27%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.144 | SwitchIndex | 0.903 | +137.27% | 0.75 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.545 | StdVisit | 0.829 | +86.41% | 0.30 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.659 | SwitchIndex | 0.935 | +77.51% | 0.32 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.344 | SwitchIndex | 0.898 | +49.57% | 1.10 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.903 | fastest | -57.85% | 0.21 |
| **PatterniaPipe** | 2.144 | +137.27% | - | 0.75 |
| StdVisit | 7.643 | +745.99% | +256.56% | 1.64 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.829 | fastest | -46.36% | 1.40 |
| SwitchIndex | 0.887 | +7.04% | -42.58% | 5.42 |
| **PatterniaPipe** | 1.545 | +86.41% | - | 0.30 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.935 | fastest | -43.67% | 1.81 |
| **PatterniaPipe** | 1.659 | +77.51% | - | 0.32 |
| StdVisit | 3.638 | +289.14% | +119.22% | 41.91 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.898 | fastest | -33.14% | 0.62 |
| StdVisit | 0.938 | +4.45% | -30.16% | 0.90 |
| **PatterniaPipe** | 1.344 | +49.57% | - | 1.10 |

