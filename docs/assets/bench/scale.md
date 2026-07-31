# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+88.59%`
- Largest Patternia gap: `ScaleN32` `+136.81%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.138 | SwitchIndex | 0.903 | +136.81% | 0.52 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.541 | SwitchIndex | 0.816 | +88.90% | 0.13 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.665 | SwitchIndex | 0.935 | +78.16% | 0.42 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.347 | SwitchIndex | 0.895 | +50.51% | 0.74 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.903 | fastest | -57.77% | 0.32 |
| **PatterniaPipe** | 2.138 | +136.81% | - | 0.52 |
| StdVisit | 7.278 | +706.10% | +240.40% | 7.38 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.816 | fastest | -47.06% | 1.73 |
| StdVisit | 0.829 | +1.60% | -46.21% | 0.31 |
| **PatterniaPipe** | 1.541 | +88.90% | - | 0.13 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.935 | fastest | -43.87% | 2.16 |
| **PatterniaPipe** | 1.665 | +78.16% | - | 0.42 |
| StdVisit | 5.748 | +515.00% | +245.19% | 0.91 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.895 | fastest | -33.56% | 0.21 |
| StdVisit | 0.935 | +4.45% | -30.60% | 0.13 |
| **PatterniaPipe** | 1.347 | +50.51% | - | 0.74 |

