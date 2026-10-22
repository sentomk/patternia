# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+87.43%`
- Largest Patternia gap: `ScaleN32` `+137.09%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.142 | SwitchIndex | 0.903 | +137.09% | 0.60 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.547 | StdVisit | 0.828 | +86.89% | 0.43 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.661 | SwitchIndex | 0.946 | +75.54% | 0.17 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.347 | SwitchIndex | 0.897 | +50.18% | 1.23 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.903 | fastest | -57.82% | 0.35 |
| **PatterniaPipe** | 2.142 | +137.09% | - | 0.60 |
| StdVisit | 3.714 | +311.19% | +73.43% | 20.98 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.828 | fastest | -46.49% | 0.63 |
| SwitchIndex | 0.877 | +5.96% | -43.30% | 5.69 |
| **PatterniaPipe** | 1.547 | +86.89% | - | 0.43 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.946 | fastest | -43.03% | 1.14 |
| **PatterniaPipe** | 1.661 | +75.54% | - | 0.17 |
| StdVisit | 4.295 | +353.97% | +158.61% | 50.29 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.897 | fastest | -33.41% | 0.49 |
| StdVisit | 0.934 | +4.17% | -30.64% | 0.24 |
| **PatterniaPipe** | 1.347 | +50.18% | - | 1.23 |

