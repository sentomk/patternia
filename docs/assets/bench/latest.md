# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `2/8`
- Average Patternia gap vs fastest: `+32.83%`
- Largest Patternia gap: `VariantMixed` `+99.40%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 3/4 | 2.130 | SwitchIndex | 1.068 | +99.40% | 0.21 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 2.100 | SwitchIndex | 1.156 | +81.69% | 3.44 | slow |
| CommandParser | PatterniaPipe | 4/4 | 2.011 | StdVisit | 1.427 | +40.89% | 1.53 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.961 | Switch | 1.600 | +22.54% | 0.85 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.819 | Switch | 1.548 | +17.56% | 0.13 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.701 | StdVisit | 1.692 | +0.55% | 0.72 | close |
| VariantAltHot | PatterniaPipe | 1/4 | 1.761 | PatterniaPipe | 1.761 | +0.00% | 0.76 | fastest |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.930 | PatterniaPipe | 1.930 | +0.00% | 1.04 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.068 | fastest | -49.85% | 0.19 |
| Sequential | 1.191 | +11.50% | -44.08% | 4.93 |
| **PatterniaPipe** | 2.130 | +99.40% | - | 0.21 |
| StdVisit | 3.188 | +198.46% | +49.68% | 0.09 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.156 | fastest | -44.96% | 0.23 |
| Sequential | 1.228 | +6.22% | -41.54% | 1.26 |
| **PatterniaPipe** | 2.100 | +81.69% | - | 3.44 |
| StdVisit | 2.313 | +100.11% | +10.13% | 0.65 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.427 | fastest | -29.02% | 0.74 |
| Switch | 1.800 | +26.14% | -10.47% | 0.86 |
| IfElse | 1.847 | +29.41% | -8.15% | 0.57 |
| **PatterniaPipe** | 2.011 | +40.89% | - | 1.53 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.600 | fastest | -18.40% | 0.21 |
| **PatterniaPipe** | 1.961 | +22.54% | - | 0.85 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.548 | fastest | -14.94% | 1.16 |
| **PatterniaPipe** | 1.819 | +17.56% | - | 0.13 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.692 | fastest | -0.55% | 0.58 |
| **PatterniaPipe** | 1.701 | +0.55% | - | 0.72 |
| Switch | 1.708 | +0.95% | +0.40% | 2.88 |
| IfElse | 2.264 | +33.86% | +33.13% | 1.45 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.761 | fastest | - | 0.76 |
| StdVisit | 1.832 | +4.00% | +4.00% | 18.67 |
| Sequential | 1.936 | +9.94% | +9.94% | 0.34 |
| SwitchIndex | 2.216 | +25.80% | +25.80% | 15.47 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.930 | fastest | - | 1.04 |
| Sequential | 2.550 | +32.14% | +32.14% | 3.56 |
| SwitchIndex | 2.638 | +36.70% | +36.70% | 0.13 |
| StdVisit | 2.860 | +48.23% | +48.23% | 13.33 |

