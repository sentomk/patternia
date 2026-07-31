# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `1/8`
- Average Patternia gap vs fastest: `+34.80%`
- Largest Patternia gap: `VariantMixed` `+99.64%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 3/4 | 2.131 | SwitchIndex | 1.068 | +99.64% | 0.04 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 2.131 | SwitchIndex | 1.162 | +83.39% | 1.24 | slow |
| CommandParser | PatterniaPipe | 4/4 | 2.107 | StdVisit | 1.422 | +48.24% | 0.26 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.959 | Switch | 1.599 | +22.55% | 0.23 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.817 | Switch | 1.524 | +19.20% | 0.15 | watch |
| VariantAltHot | PatterniaPipe | 2/4 | 1.757 | StdVisit | 1.682 | +4.50% | 0.02 | close |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.706 | StdVisit | 1.691 | +0.89% | 1.30 | close |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.934 | PatterniaPipe | 1.934 | +0.00% | 0.06 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.068 | fastest | -49.91% | 0.12 |
| Sequential | 1.166 | +9.19% | -45.30% | 0.11 |
| **PatterniaPipe** | 2.131 | +99.64% | - | 0.04 |
| StdVisit | 3.171 | +197.02% | +48.78% | 0.41 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.162 | fastest | -45.47% | 1.35 |
| Sequential | 1.241 | +6.79% | -41.77% | 0.16 |
| **PatterniaPipe** | 2.131 | +83.39% | - | 1.24 |
| StdVisit | 2.308 | +98.59% | +8.29% | 0.06 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.422 | fastest | -32.54% | 0.82 |
| Switch | 1.792 | +26.05% | -14.97% | 0.81 |
| IfElse | 1.930 | +35.73% | -8.44% | 2.21 |
| **PatterniaPipe** | 2.107 | +48.24% | - | 0.26 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.599 | fastest | -18.40% | 0.45 |
| **PatterniaPipe** | 1.959 | +22.55% | - | 0.23 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.524 | fastest | -16.11% | 1.82 |
| **PatterniaPipe** | 1.817 | +19.20% | - | 0.15 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.682 | fastest | -4.31% | 14.28 |
| **PatterniaPipe** | 1.757 | +4.50% | - | 0.02 |
| Sequential | 2.466 | +46.63% | +40.31% | 0.14 |
| SwitchIndex | 2.993 | +77.97% | +70.31% | 0.08 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.691 | fastest | -0.88% | 0.53 |
| **PatterniaPipe** | 1.706 | +0.89% | - | 1.30 |
| Switch | 1.734 | +2.54% | +1.64% | 2.61 |
| IfElse | 2.240 | +32.42% | +31.25% | 2.28 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.934 | fastest | - | 0.06 |
| StdVisit | 2.993 | +54.78% | +54.78% | 0.06 |
| Sequential | 2.996 | +54.90% | +54.90% | 0.34 |
| SwitchIndex | 3.170 | +63.89% | +63.89% | 0.08 |

