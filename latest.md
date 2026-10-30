# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `2/8`
- Average Patternia gap vs fastest: `+33.06%`
- Largest Patternia gap: `VariantMixed` `+99.63%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 3/4 | 2.134 | SwitchIndex | 1.069 | +99.63% | 0.22 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 2.113 | SwitchIndex | 1.158 | +82.48% | 3.04 | slow |
| CommandParser | PatterniaPipe | 4/4 | 2.006 | StdVisit | 1.416 | +41.71% | 0.59 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.961 | Switch | 1.605 | +22.17% | 0.17 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.816 | Switch | 1.546 | +17.52% | 0.10 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.700 | StdVisit | 1.684 | +0.98% | 0.54 | close |
| VariantAltHot | PatterniaPipe | 1/4 | 1.758 | PatterniaPipe | 1.758 | +0.00% | 0.09 | fastest |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.937 | PatterniaPipe | 1.937 | +0.00% | 0.36 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.069 | fastest | -49.91% | 0.57 |
| Sequential | 1.167 | +9.16% | -45.32% | 0.37 |
| **PatterniaPipe** | 2.134 | +99.63% | - | 0.22 |
| StdVisit | 3.173 | +196.90% | +48.73% | 0.19 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.158 | fastest | -45.20% | 0.40 |
| Sequential | 1.220 | +5.35% | -42.27% | 0.24 |
| **PatterniaPipe** | 2.113 | +82.48% | - | 3.04 |
| StdVisit | 2.312 | +99.69% | +9.44% | 0.17 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.416 | fastest | -29.43% | 0.87 |
| Switch | 1.789 | +26.35% | -10.84% | 0.67 |
| IfElse | 1.867 | +31.90% | -6.92% | 0.97 |
| **PatterniaPipe** | 2.006 | +41.71% | - | 0.59 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.605 | fastest | -18.15% | 0.90 |
| **PatterniaPipe** | 1.961 | +22.17% | - | 0.17 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.546 | fastest | -14.91% | 1.36 |
| **PatterniaPipe** | 1.816 | +17.52% | - | 0.10 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.684 | fastest | -0.97% | 0.38 |
| **PatterniaPipe** | 1.700 | +0.98% | - | 0.54 |
| Switch | 1.717 | +1.97% | +0.98% | 3.29 |
| IfElse | 2.219 | +31.79% | +30.51% | 1.88 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.758 | fastest | - | 0.09 |
| StdVisit | 2.272 | +29.26% | +29.26% | 23.43 |
| Sequential | 2.994 | +70.28% | +70.28% | 0.08 |
| SwitchIndex | 2.995 | +70.34% | +70.34% | 0.12 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.937 | fastest | - | 0.36 |
| Sequential | 2.077 | +7.23% | +7.23% | 3.16 |
| StdVisit | 2.994 | +54.57% | +54.57% | 0.04 |
| SwitchIndex | 3.136 | +61.90% | +61.90% | 4.72 |

