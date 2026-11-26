# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `0/8`
- Average Patternia gap vs fastest: `+10.45%`
- Largest Patternia gap: `VariantMixedGuarded` `+21.90%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixedGuarded | PatterniaPipe | 4/4 | 1.116 | SwitchIndex | 0.916 | +21.90% | 0.59 | slow |
| VariantAltHotGuarded | PatterniaPipe | 3/4 | 0.690 | Sequential | 0.587 | +17.45% | 0.81 | watch |
| CommandParser | PatterniaPipe | 2/4 | 1.340 | StdVisit | 1.166 | +14.89% | 0.16 | watch |
| VariantAltHot | PatterniaPipe | 3/4 | 0.523 | SwitchIndex | 0.475 | +10.03% | 0.36 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.341 | Switch | 1.241 | +7.98% | 0.26 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.423 | StdVisit | 1.358 | +4.78% | 0.57 | close |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.501 | Switch | 1.438 | +4.41% | 0.38 | close |
| VariantMixed | PatterniaPipe | 3/4 | 0.801 | StdVisit | 0.784 | +2.15% | 0.30 | close |

---

## Per-Scenario Details
### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.916 | fastest | -17.97% | 0.14 |
| Sequential | 0.997 | +8.86% | -10.70% | 0.20 |
| StdVisit | 1.004 | +9.58% | -10.11% | 0.17 |
| **PatterniaPipe** | 1.116 | +21.90% | - | 0.59 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Sequential | 0.587 | fastest | -14.85% | 0.30 |
| SwitchIndex | 0.588 | +0.17% | -14.71% | 0.17 |
| **PatterniaPipe** | 0.690 | +17.45% | - | 0.81 |
| StdVisit | 0.750 | +27.81% | +8.82% | 0.21 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.166 | fastest | -12.96% | 0.73 |
| **PatterniaPipe** | 1.340 | +14.89% | - | 0.16 |
| Switch | 1.390 | +19.21% | +3.76% | 0.16 |
| IfElse | 1.610 | +38.06% | +20.17% | 2.75 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.475 | fastest | -9.12% | 0.60 |
| StdVisit | 0.520 | +9.38% | -0.60% | 0.34 |
| **PatterniaPipe** | 0.523 | +10.03% | - | 0.36 |
| Sequential | 0.656 | +38.11% | +25.52% | 1.48 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.241 | fastest | -7.39% | 0.22 |
| **PatterniaPipe** | 1.341 | +7.98% | - | 0.26 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.358 | fastest | -4.56% | 0.19 |
| **PatterniaPipe** | 1.423 | +4.78% | - | 0.57 |
| IfElse | 1.666 | +22.65% | +17.05% | 0.36 |
| Switch | 1.749 | +28.75% | +22.87% | 0.88 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.438 | fastest | -4.22% | 0.58 |
| **PatterniaPipe** | 1.501 | +4.41% | - | 0.38 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 0.784 | fastest | -2.11% | 0.10 |
| SwitchIndex | 0.785 | +0.16% | -1.95% | 0.18 |
| **PatterniaPipe** | 0.801 | +2.15% | - | 0.30 |
| Sequential | 0.907 | +15.65% | +13.22% | 0.31 |

