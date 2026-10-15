# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+87.65%`
- Largest Patternia gap: `ScaleN32` `+135.88%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.146 | SwitchIndex | 0.910 | +135.88% | 0.43 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.550 | StdVisit | 0.824 | +87.97% | 0.36 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.660 | SwitchIndex | 0.948 | +75.20% | 0.19 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.355 | SwitchIndex | 0.894 | +51.54% | 0.84 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.910 | fastest | -57.61% | 0.61 |
| **PatterniaPipe** | 2.146 | +135.88% | - | 0.43 |
| StdVisit | 8.135 | +794.41% | +279.18% | 3.11 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.824 | fastest | -46.80% | 0.19 |
| SwitchIndex | 0.894 | +8.46% | -42.30% | 4.48 |
| **PatterniaPipe** | 1.550 | +87.97% | - | 0.36 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.948 | fastest | -42.92% | 1.04 |
| **PatterniaPipe** | 1.660 | +75.20% | - | 0.19 |
| StdVisit | 5.647 | +495.97% | +240.17% | 1.30 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.894 | fastest | -34.01% | 0.10 |
| StdVisit | 0.934 | +4.42% | -31.09% | 0.09 |
| **PatterniaPipe** | 1.355 | +51.54% | - | 0.84 |

