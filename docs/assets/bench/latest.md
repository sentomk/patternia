# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `2/8`
- Average Patternia gap vs fastest: `+33.05%`
- Largest Patternia gap: `VariantMixed` `+99.32%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 3/4 | 2.127 | SwitchIndex | 1.067 | +99.32% | 0.28 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 2.134 | SwitchIndex | 1.155 | +84.70% | 0.89 | slow |
| CommandParser | PatterniaPipe | 4/4 | 2.103 | StdVisit | 1.453 | +44.78% | 0.79 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.964 | Switch | 1.616 | +21.55% | 1.12 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.819 | Switch | 1.609 | +13.01% | 0.17 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.710 | StdVisit | 1.692 | +1.05% | 0.99 | close |
| VariantAltHot | PatterniaPipe | 1/4 | 1.757 | PatterniaPipe | 1.757 | +0.00% | 0.07 | fastest |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.934 | PatterniaPipe | 1.934 | +0.00% | 0.09 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.067 | fastest | -49.83% | 0.20 |
| Sequential | 1.168 | +9.38% | -45.12% | 0.59 |
| **PatterniaPipe** | 2.127 | +99.32% | - | 0.28 |
| StdVisit | 3.178 | +197.72% | +49.37% | 0.33 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.155 | fastest | -45.86% | 0.57 |
| Sequential | 1.222 | +5.74% | -42.75% | 1.43 |
| **PatterniaPipe** | 2.134 | +84.70% | - | 0.89 |
| StdVisit | 2.309 | +99.90% | +8.23% | 0.34 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.453 | fastest | -30.93% | 2.06 |
| Switch | 1.810 | +24.60% | -13.94% | 0.79 |
| IfElse | 1.871 | +28.82% | -11.03% | 0.76 |
| **PatterniaPipe** | 2.103 | +44.78% | - | 0.79 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.616 | fastest | -17.73% | 0.56 |
| **PatterniaPipe** | 1.964 | +21.55% | - | 1.12 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.609 | fastest | -11.51% | 1.77 |
| **PatterniaPipe** | 1.819 | +13.01% | - | 0.17 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.692 | fastest | -1.04% | 0.56 |
| **PatterniaPipe** | 1.710 | +1.05% | - | 0.99 |
| Switch | 1.722 | +1.73% | +0.67% | 4.00 |
| IfElse | 2.147 | +26.85% | +25.53% | 1.87 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.757 | fastest | - | 0.07 |
| Sequential | 2.461 | +40.04% | +40.04% | 0.07 |
| StdVisit | 2.462 | +40.11% | +40.11% | 22.54 |
| SwitchIndex | 2.993 | +70.30% | +70.30% | 0.38 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.934 | fastest | - | 0.09 |
| SwitchIndex | 2.636 | +36.32% | +36.32% | 0.05 |
| StdVisit | 2.988 | +54.52% | +54.52% | 0.04 |
| Sequential | 2.989 | +54.54% | +54.54% | 0.08 |

