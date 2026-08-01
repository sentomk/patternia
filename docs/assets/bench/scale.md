# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+90.38%`
- Largest Patternia gap: `ScaleN32` `+143.24%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.199 | SwitchIndex | 0.904 | +143.24% | 1.70 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.541 | SwitchIndex | 0.817 | +88.64% | 0.19 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.665 | SwitchIndex | 0.928 | +79.37% | 0.26 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.345 | SwitchIndex | 0.895 | +50.27% | 0.70 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.904 | fastest | -58.89% | 0.47 |
| **PatterniaPipe** | 2.199 | +143.24% | - | 1.70 |
| StdVisit | 8.835 | +877.21% | +301.74% | 0.29 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.817 | fastest | -46.99% | 2.08 |
| StdVisit | 0.821 | +0.55% | -46.70% | 0.39 |
| **PatterniaPipe** | 1.541 | +88.64% | - | 0.19 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.928 | fastest | -44.25% | 1.67 |
| **PatterniaPipe** | 1.665 | +79.37% | - | 0.26 |
| StdVisit | 6.926 | +646.21% | +316.01% | 4.56 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.895 | fastest | -33.45% | 0.19 |
| StdVisit | 0.933 | +4.31% | -30.58% | 0.19 |
| **PatterniaPipe** | 1.345 | +50.27% | - | 0.70 |

