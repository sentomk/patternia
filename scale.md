# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+77.63%`
- Largest Patternia gap: `ScaleN32` `+116.03%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.180 | SwitchIndex | 1.009 | +116.03% | 0.80 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.744 | StdVisit | 0.953 | +83.07% | 2.02 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.759 | SwitchIndex | 1.014 | +73.40% | 1.32 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.399 | SwitchIndex | 1.013 | +38.02% | 0.61 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.009 | fastest | -53.71% | 0.13 |
| **PatterniaPipe** | 2.180 | +116.03% | - | 0.80 |
| StdVisit | 6.706 | +564.42% | +207.56% | 1.69 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.953 | fastest | -45.38% | 0.10 |
| SwitchIndex | 1.017 | +6.74% | -41.70% | 5.33 |
| **PatterniaPipe** | 1.744 | +83.07% | - | 2.02 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.014 | fastest | -42.33% | 0.18 |
| **PatterniaPipe** | 1.759 | +73.40% | - | 1.32 |
| StdVisit | 3.134 | +209.01% | +78.21% | 0.06 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.013 | fastest | -27.54% | 0.22 |
| StdVisit | 1.054 | +4.01% | -24.64% | 0.19 |
| **PatterniaPipe** | 1.399 | +38.02% | - | 0.61 |

