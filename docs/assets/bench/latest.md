# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `1/8`
- Average Patternia gap vs fastest: `+24.60%`
- Largest Patternia gap: `VariantMixed` `+98.42%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.998 | SwitchIndex | 1.007 | +98.42% | 0.22 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.995 | Sequential | 1.427 | +39.81% | 0.08 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.771 | StdVisit | 1.401 | +26.45% | 0.06 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.690 | Switch | 1.493 | +13.25% | 0.46 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.622 | Switch | 1.447 | +12.07% | 0.27 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.617 | StdVisit | 1.516 | +6.68% | 0.21 | watch |
| VariantAltHot | PatterniaPipe | 2/4 | 1.558 | StdVisit | 1.556 | +0.11% | 0.27 | close |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.715 | PatterniaPipe | 1.715 | +0.00% | 0.61 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.007 | fastest | -49.60% | 0.26 |
| Sequential | 1.168 | +16.03% | -41.53% | 0.12 |
| StdVisit | 1.991 | +97.69% | -0.37% | 0.10 |
| **PatterniaPipe** | 1.998 | +98.42% | - | 0.22 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Sequential | 1.427 | fastest | -28.48% | 0.28 |
| SwitchIndex | 1.429 | +0.13% | -28.39% | 1.73 |
| **PatterniaPipe** | 1.995 | +39.81% | - | 0.08 |
| StdVisit | 2.153 | +50.90% | +7.93% | 0.15 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.401 | fastest | -20.92% | 0.62 |
| **PatterniaPipe** | 1.771 | +26.45% | - | 0.06 |
| IfElse | 2.202 | +57.25% | +24.35% | 0.56 |
| Switch | 2.248 | +60.48% | +26.91% | 0.12 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.493 | fastest | -11.70% | 0.19 |
| **PatterniaPipe** | 1.690 | +13.25% | - | 0.46 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.447 | fastest | -10.77% | 1.35 |
| **PatterniaPipe** | 1.622 | +12.07% | - | 0.27 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.516 | fastest | -6.26% | 0.20 |
| Switch | 1.592 | +4.99% | -1.58% | 0.47 |
| **PatterniaPipe** | 1.617 | +6.68% | - | 0.21 |
| IfElse | 1.937 | +27.75% | +19.75% | 5.47 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.556 | fastest | -0.11% | 0.06 |
| **PatterniaPipe** | 1.558 | +0.11% | - | 0.27 |
| SwitchIndex | 1.712 | +9.99% | +9.87% | 0.08 |
| Sequential | 1.735 | +11.49% | +11.36% | 5.93 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.715 | fastest | - | 0.61 |
| StdVisit | 1.723 | +0.45% | +0.45% | 0.84 |
| SwitchIndex | 1.868 | +8.95% | +8.95% | 0.10 |
| Sequential | 2.336 | +36.18% | +36.18% | 0.20 |

