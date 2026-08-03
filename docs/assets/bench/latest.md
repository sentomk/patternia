# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `2/8`
- Average Patternia gap vs fastest: `+31.77%`
- Largest Patternia gap: `VariantMixed` `+100.05%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 3/4 | 2.135 | SwitchIndex | 1.067 | +100.05% | 0.25 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 2.097 | SwitchIndex | 1.210 | +73.23% | 3.68 | slow |
| CommandParser | PatterniaPipe | 4/4 | 2.081 | StdVisit | 1.449 | +43.59% | 2.10 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.963 | Switch | 1.599 | +22.79% | 1.07 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.825 | Switch | 1.602 | +13.94% | 0.99 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.713 | StdVisit | 1.703 | +0.57% | 0.94 | close |
| VariantAltHot | PatterniaPipe | 1/4 | 1.757 | PatterniaPipe | 1.757 | +0.00% | 0.13 | fastest |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.934 | PatterniaPipe | 1.934 | +0.00% | 0.16 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.067 | fastest | -50.01% | 0.04 |
| Sequential | 1.167 | +9.36% | -45.33% | 0.42 |
| **PatterniaPipe** | 2.135 | +100.05% | - | 0.25 |
| StdVisit | 2.137 | +100.29% | +0.12% | 0.49 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.210 | fastest | -42.27% | 3.31 |
| Sequential | 1.217 | +0.53% | -41.97% | 0.16 |
| **PatterniaPipe** | 2.097 | +73.23% | - | 3.68 |
| StdVisit | 2.310 | +90.85% | +10.17% | 0.41 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.449 | fastest | -30.36% | 0.91 |
| Switch | 1.819 | +25.55% | -12.57% | 0.60 |
| IfElse | 1.843 | +27.20% | -11.41% | 0.96 |
| **PatterniaPipe** | 2.081 | +43.59% | - | 2.10 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.599 | fastest | -18.56% | 0.40 |
| **PatterniaPipe** | 1.963 | +22.79% | - | 1.07 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.602 | fastest | -12.24% | 1.83 |
| **PatterniaPipe** | 1.825 | +13.94% | - | 0.99 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.703 | fastest | -0.57% | 0.91 |
| **PatterniaPipe** | 1.713 | +0.57% | - | 0.94 |
| Switch | 1.792 | +5.22% | +4.62% | 3.67 |
| IfElse | 2.185 | +28.26% | +27.53% | 2.50 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.757 | fastest | - | 0.13 |
| Sequential | 2.410 | +37.14% | +37.14% | 25.67 |
| SwitchIndex | 2.465 | +40.28% | +40.28% | 0.40 |
| StdVisit | 2.821 | +60.53% | +60.53% | 0.46 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.934 | fastest | - | 0.16 |
| SwitchIndex | 2.638 | +36.37% | +36.37% | 0.28 |
| StdVisit | 2.990 | +54.59% | +54.59% | 0.09 |
| Sequential | 2.992 | +54.68% | +54.68% | 0.05 |

