# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `2/8`
- Average Patternia gap vs fastest: `+31.50%`
- Largest Patternia gap: `VariantMixed` `+99.91%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 3/4 | 2.136 | SwitchIndex | 1.069 | +99.91% | 0.38 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 2.034 | SwitchIndex | 1.154 | +76.27% | 4.26 | slow |
| CommandParser | PatterniaPipe | 4/4 | 2.012 | StdVisit | 1.443 | +39.40% | 1.35 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.963 | Switch | 1.612 | +21.75% | 0.89 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.820 | Switch | 1.592 | +14.28% | 0.29 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.699 | StdVisit | 1.693 | +0.37% | 0.73 | close |
| VariantAltHot | PatterniaPipe | 1/4 | 1.761 | PatterniaPipe | 1.761 | +0.00% | 0.48 | fastest |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.934 | PatterniaPipe | 1.934 | +0.00% | 0.08 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.069 | fastest | -49.98% | 0.42 |
| Sequential | 1.166 | +9.16% | -45.40% | 0.24 |
| **PatterniaPipe** | 2.136 | +99.91% | - | 0.38 |
| StdVisit | 3.187 | +198.23% | +49.18% | 0.03 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.154 | fastest | -43.27% | 0.15 |
| Sequential | 1.216 | +5.42% | -40.19% | 0.17 |
| **PatterniaPipe** | 2.034 | +76.27% | - | 4.26 |
| StdVisit | 2.308 | +100.05% | +13.49% | 0.36 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.443 | fastest | -28.26% | 1.22 |
| Switch | 1.807 | +25.17% | -10.21% | 0.52 |
| IfElse | 1.921 | +33.13% | -4.50% | 2.66 |
| **PatterniaPipe** | 2.012 | +39.40% | - | 1.35 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.612 | fastest | -17.86% | 0.13 |
| **PatterniaPipe** | 1.963 | +21.75% | - | 0.89 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.592 | fastest | -12.50% | 2.41 |
| **PatterniaPipe** | 1.820 | +14.28% | - | 0.29 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.693 | fastest | -0.37% | 0.58 |
| **PatterniaPipe** | 1.699 | +0.37% | - | 0.73 |
| Switch | 1.750 | +3.37% | +2.98% | 3.61 |
| IfElse | 2.224 | +31.35% | +30.86% | 2.27 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.761 | fastest | - | 0.48 |
| SwitchIndex | 2.074 | +17.80% | +17.80% | 16.73 |
| StdVisit | 2.818 | +60.03% | +60.03% | 0.42 |
| Sequential | 2.992 | +69.92% | +69.92% | 0.16 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.934 | fastest | - | 0.08 |
| StdVisit | 2.478 | +28.12% | +28.12% | 1.05 |
| Sequential | 2.990 | +54.61% | +54.61% | 0.06 |
| SwitchIndex | 3.165 | +63.66% | +63.66% | 0.05 |

