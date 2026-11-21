# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `0/8`
- Average Patternia gap vs fastest: `+11.17%`
- Largest Patternia gap: `PacketMixed` `+27.48%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 1.526 | Switch | 1.197 | +27.48% | 1.21 | slow |
| VariantAltHotGuarded | PatterniaPipe | 3/4 | 0.692 | SwitchIndex | 0.588 | +17.78% | 1.02 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.422 | StdVisit | 1.252 | +13.63% | 0.74 | watch |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.588 | Switch | 1.420 | +11.78% | 0.77 | watch |
| VariantAltHot | PatterniaPipe | 3/4 | 0.523 | SwitchIndex | 0.479 | +9.37% | 0.13 | watch |
| CommandParser | PatterniaPipe | 2/4 | 1.338 | Switch | 1.271 | +5.22% | 0.06 | watch |
| VariantMixed | PatterniaPipe | 2/4 | 0.804 | StdVisit | 0.786 | +2.31% | 0.22 | close |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.026 | StdVisit | 1.008 | +1.77% | 0.13 | close |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.197 | fastest | -21.56% | 0.05 |
| **PatterniaPipe** | 1.526 | +27.48% | - | 1.21 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.588 | fastest | -15.10% | 0.25 |
| Sequential | 0.588 | +0.06% | -15.05% | 0.18 |
| **PatterniaPipe** | 0.692 | +17.78% | - | 1.02 |
| StdVisit | 0.754 | +28.25% | +8.89% | 1.06 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.252 | fastest | -12.00% | 0.45 |
| **PatterniaPipe** | 1.422 | +13.63% | - | 0.74 |
| Switch | 1.620 | +29.45% | +13.92% | 0.34 |
| IfElse | 1.718 | +37.29% | +20.82% | 0.39 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.420 | fastest | -10.54% | 0.32 |
| **PatterniaPipe** | 1.588 | +11.78% | - | 0.77 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.479 | fastest | -8.57% | 0.38 |
| StdVisit | 0.519 | +8.37% | -0.91% | 0.38 |
| **PatterniaPipe** | 0.523 | +9.37% | - | 0.13 |
| Sequential | 0.656 | +37.08% | +25.33% | 4.46 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.271 | fastest | -4.96% | 0.18 |
| **PatterniaPipe** | 1.338 | +5.22% | - | 0.06 |
| StdVisit | 1.473 | +15.86% | +10.11% | 0.57 |
| IfElse | 1.688 | +32.79% | +26.20% | 0.37 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.786 | fastest | -2.26% | 0.16 |
| **PatterniaPipe** | 0.804 | +2.31% | - | 0.22 |
| SwitchIndex | 0.826 | +5.20% | +2.82% | 0.57 |
| Sequential | 0.891 | +13.47% | +10.91% | 0.31 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.008 | fastest | -1.74% | 0.49 |
| SwitchIndex | 1.024 | +1.55% | -0.22% | 0.43 |
| **PatterniaPipe** | 1.026 | +1.77% | - | 0.13 |
| Sequential | 1.082 | +7.35% | +5.48% | 0.39 |

