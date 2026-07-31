# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+80.46%`
- Largest Patternia gap: `ScaleN32` `+127.14%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN32 | PatterniaPipe | 2/3 | 2.297 | SwitchIndex | 1.011 | +127.14% | 2.22 | slow |
| ScaleN8 | PatterniaPipe | 3/3 | 1.774 | StdVisit | 0.954 | +85.86% | 2.31 | slow |
| ScaleN16 | PatterniaPipe | 2/3 | 1.737 | SwitchIndex | 1.015 | +71.15% | 1.01 | slow |
| ScaleN4 | PatterniaPipe | 3/3 | 1.396 | SwitchIndex | 1.014 | +37.69% | 0.40 | slow |

---

## Per-Scenario Details
### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.011 | fastest | -55.97% | 0.60 |
| **PatterniaPipe** | 2.297 | +127.14% | - | 2.22 |
| StdVisit | 6.691 | +561.52% | +191.25% | 1.30 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.954 | fastest | -46.20% | 0.48 |
| SwitchIndex | 0.978 | +2.46% | -44.87% | 4.04 |
| **PatterniaPipe** | 1.774 | +85.86% | - | 2.31 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.015 | fastest | -41.57% | 0.51 |
| **PatterniaPipe** | 1.737 | +71.15% | - | 1.01 |
| StdVisit | 3.129 | +208.35% | +80.16% | 0.26 |

### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.014 | fastest | -27.38% | 0.16 |
| StdVisit | 1.052 | +3.82% | -24.60% | 0.22 |
| **PatterniaPipe** | 1.396 | +37.69% | - | 0.40 |

