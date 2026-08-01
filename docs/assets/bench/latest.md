# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `0/8`
- Average Patternia gap vs fastest: `+24.29%`
- Largest Patternia gap: `VariantMixed` `+100.52%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 2.000 | SwitchIndex | 0.997 | +100.52% | 0.38 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.847 | Sequential | 1.423 | +29.76% | 0.36 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.795 | StdVisit | 1.399 | +28.30% | 2.18 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.622 | Switch | 1.407 | +15.28% | 0.32 | watch |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.682 | Switch | 1.476 | +13.94% | 0.22 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.614 | StdVisit | 1.516 | +6.44% | 0.18 | watch |
| VariantAltHot | PatterniaPipe | 2/4 | 1.557 | StdVisit | 1.557 | +0.02% | 0.14 | close |
| VariantAltHotGuarded | PatterniaPipe | 2/4 | 1.713 | StdVisit | 1.713 | +0.01% | 0.08 | close |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.997 | fastest | -50.13% | 0.37 |
| Sequential | 1.168 | +17.09% | -41.60% | 0.20 |
| StdVisit | 1.994 | +99.96% | -0.28% | 0.46 |
| **PatterniaPipe** | 2.000 | +100.52% | - | 0.38 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Sequential | 1.423 | fastest | -22.93% | 0.68 |
| SwitchIndex | 1.466 | +3.02% | -20.61% | 0.94 |
| **PatterniaPipe** | 1.847 | +29.76% | - | 0.36 |
| StdVisit | 2.152 | +51.18% | +16.51% | 0.08 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.399 | fastest | -22.06% | 0.52 |
| **PatterniaPipe** | 1.795 | +28.30% | - | 2.18 |
| IfElse | 2.130 | +52.25% | +18.66% | 0.64 |
| Switch | 2.248 | +60.71% | +25.26% | 0.17 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.407 | fastest | -13.26% | 1.68 |
| **PatterniaPipe** | 1.622 | +15.28% | - | 0.32 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.476 | fastest | -12.24% | 0.45 |
| **PatterniaPipe** | 1.682 | +13.94% | - | 0.22 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.516 | fastest | -6.05% | 0.26 |
| Switch | 1.606 | +5.88% | -0.53% | 0.39 |
| **PatterniaPipe** | 1.614 | +6.44% | - | 0.18 |
| IfElse | 2.040 | +34.52% | +26.38% | 3.83 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.557 | fastest | -0.02% | 0.08 |
| **PatterniaPipe** | 1.557 | +0.02% | - | 0.14 |
| SwitchIndex | 1.713 | +10.04% | +10.01% | 0.10 |
| Sequential | 1.715 | +10.12% | +10.10% | 0.46 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.713 | fastest | -0.01% | 0.08 |
| **PatterniaPipe** | 1.713 | +0.01% | - | 0.08 |
| Sequential | 1.868 | +9.05% | +9.04% | 0.06 |
| SwitchIndex | 1.869 | +9.08% | +9.07% | 0.11 |

