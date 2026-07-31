# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+80.00%`
- Largest Patternia gap: `ScaleN32` `+124.18%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.261 | SwitchIndex | 1.009 | +124.18% | 1.46 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.779 | StdVisit | 0.952 | +86.79% | 2.45 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.735 | SwitchIndex | 1.013 | +71.34% | 0.85 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.395 | SwitchIndex | 1.013 | +37.69% | 0.21 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.009 | fastest | -55.39% | 0.17 |
| **PatterniaPipe** | 2.261 | +124.18% | - | 1.46 |
| StdVisit | 6.585 | +552.90% | +191.25% | 0.12 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.952 | fastest | -46.46% | 0.19 |
| SwitchIndex | 0.982 | +3.13% | -44.79% | 3.74 |
| **PatterniaPipe** | 1.779 | +86.79% | - | 2.45 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.013 | fastest | -41.64% | 0.11 |
| **PatterniaPipe** | 1.735 | +71.34% | - | 0.85 |
| StdVisit | 3.125 | +208.53% | +80.07% | 0.04 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.013 | fastest | -27.37% | 0.82 |
| StdVisit | 1.052 | +3.84% | -24.58% | 0.13 |
| **PatterniaPipe** | 1.395 | +37.69% | - | 0.21 |

