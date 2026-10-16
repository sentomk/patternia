# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `2/8`
- Average Patternia gap vs fastest: `+32.33%`
- Largest Patternia gap: `VariantMixed` `+100.24%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 3/4 | 1.658 | SwitchIndex | 0.828 | +100.24% | 0.52 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.635 | SwitchIndex | 0.922 | +77.35% | 3.09 | slow |
| CommandParser | PatterniaPipe | 4/4 | 1.561 | StdVisit | 1.107 | +41.02% | 1.60 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.517 | Switch | 1.255 | +20.90% | 0.19 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.408 | Switch | 1.206 | +16.77% | 0.12 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.347 | StdVisit | 1.315 | +2.39% | 1.35 | close |
| VariantAltHot | PatterniaPipe | 1/4 | 1.366 | PatterniaPipe | 1.366 | +0.00% | 0.36 | fastest |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.501 | PatterniaPipe | 1.501 | +0.00% | 0.32 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.828 | fastest | -50.06% | 0.08 |
| Sequential | 0.905 | +9.32% | -45.41% | 0.60 |
| **PatterniaPipe** | 1.658 | +100.24% | - | 0.52 |
| StdVisit | 2.461 | +197.18% | +48.41% | 0.24 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.922 | fastest | -43.61% | 3.16 |
| Sequential | 0.959 | +4.07% | -41.32% | 0.73 |
| **PatterniaPipe** | 1.635 | +77.35% | - | 3.09 |
| StdVisit | 1.788 | +93.94% | +9.36% | 0.48 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.107 | fastest | -29.09% | 0.79 |
| Switch | 1.392 | +25.68% | -10.88% | 0.62 |
| IfElse | 1.463 | +32.16% | -6.28% | 1.93 |
| **PatterniaPipe** | 1.561 | +41.02% | - | 1.60 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.255 | fastest | -17.29% | 0.81 |
| **PatterniaPipe** | 1.517 | +20.90% | - | 0.19 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.206 | fastest | -14.36% | 1.75 |
| **PatterniaPipe** | 1.408 | +16.77% | - | 0.12 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.315 | fastest | -2.33% | 0.70 |
| **PatterniaPipe** | 1.347 | +2.39% | - | 1.35 |
| Switch | 1.352 | +2.80% | +0.40% | 3.46 |
| IfElse | 1.741 | +32.39% | +29.31% | 2.46 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.366 | fastest | - | 0.36 |
| StdVisit | 1.677 | +22.82% | +22.82% | 7.41 |
| Sequential | 1.914 | +40.13% | +40.13% | 0.24 |
| SwitchIndex | 2.322 | +70.00% | +70.00% | 0.04 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.501 | fastest | - | 0.32 |
| SwitchIndex | 1.636 | +8.98% | +8.98% | 0.06 |
| StdVisit | 1.955 | +30.20% | +30.20% | 21.47 |
| Sequential | 1.961 | +30.59% | +30.59% | 13.20 |

