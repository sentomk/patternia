# Patternia Benchmark Report

- Source: `bench_results/ptn_bench_scale.json`
- Scenarios: `4`
- Patternia fastest: `0/4`
- Average Patternia gap vs fastest: `+7.06%`
- Largest Patternia gap: `ScaleN4` `+15.04%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| ScaleN4 | PatterniaPipe | 3/3 | 0.654 | SwitchIndex | 0.568 | +15.04% | 0.14 | watch |
| ScaleN8 | PatterniaPipe | 3/3 | 0.776 | StdVisit | 0.694 | +11.81% | 0.19 | watch |
| ScaleN32 | PatterniaPipe | 2/3 | 0.697 | SwitchIndex | 0.690 | +1.10% | 1.07 | close |
| ScaleN16 | PatterniaPipe | 2/3 | 0.554 | SwitchIndex | 0.552 | +0.31% | 0.16 | close |

---

## Per-Scenario Details
### ScaleN4

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.568 | fastest | -13.07% | 0.19 |
| StdVisit | 0.645 | +13.57% | -1.27% | 0.52 |
| **PatterniaPipe** | 0.654 | +15.04% | - | 0.14 |

### ScaleN8

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.694 | fastest | -10.56% | 0.15 |
| SwitchIndex | 0.775 | +11.65% | -0.15% | 0.09 |
| **PatterniaPipe** | 0.776 | +11.81% | - | 0.19 |

### ScaleN32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.690 | fastest | -1.09% | 1.56 |
| **PatterniaPipe** | 0.697 | +1.10% | - | 1.07 |
| StdVisit | 1.562 | +126.42% | +123.95% | 0.33 |

### ScaleN16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.552 | fastest | -0.31% | 0.29 |
| **PatterniaPipe** | 0.554 | +0.31% | - | 0.16 |
| StdVisit | 1.602 | +190.35% | +189.46% | 0.57 |

