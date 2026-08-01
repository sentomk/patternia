# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `0/8`
- Average Patternia gap vs fastest: `+25.32%`
- Largest Patternia gap: `VariantMixed` `+105.38%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.997 | SwitchIndex | 0.973 | +105.38% | 0.45 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.995 | Sequential | 1.429 | +39.58% | 0.18 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.772 | StdVisit | 1.403 | +26.34% | 0.15 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.681 | Switch | 1.493 | +12.58% | 0.56 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.621 | Switch | 1.456 | +11.36% | 0.26 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.625 | StdVisit | 1.515 | +7.23% | 0.52 | watch |
| VariantAltHotGuarded | PatterniaPipe | 2/4 | 1.714 | StdVisit | 1.712 | +0.07% | 0.40 | close |
| VariantAltHot | PatterniaPipe | 2/4 | 1.557 | StdVisit | 1.556 | +0.01% | 0.09 | close |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.973 | fastest | -51.31% | 4.63 |
| Sequential | 1.165 | +19.84% | -41.65% | 0.17 |
| StdVisit | 1.994 | +105.01% | -0.18% | 0.59 |
| **PatterniaPipe** | 1.997 | +105.38% | - | 0.45 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Sequential | 1.429 | fastest | -28.35% | 0.37 |
| SwitchIndex | 1.430 | +0.05% | -28.32% | 1.41 |
| **PatterniaPipe** | 1.995 | +39.58% | - | 0.18 |
| StdVisit | 2.151 | +50.46% | +7.79% | 0.07 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.403 | fastest | -20.85% | 0.49 |
| **PatterniaPipe** | 1.772 | +26.34% | - | 0.15 |
| IfElse | 2.187 | +55.86% | +23.37% | 1.67 |
| Switch | 2.252 | +60.56% | +27.09% | 0.24 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.493 | fastest | -11.18% | 0.24 |
| **PatterniaPipe** | 1.681 | +12.58% | - | 0.56 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.456 | fastest | -10.20% | 1.86 |
| **PatterniaPipe** | 1.621 | +11.36% | - | 0.26 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.515 | fastest | -6.74% | 0.25 |
| Switch | 1.581 | +4.35% | -2.68% | 0.38 |
| **PatterniaPipe** | 1.625 | +7.23% | - | 0.52 |
| IfElse | 1.864 | +23.04% | +14.75% | 0.17 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.712 | fastest | -0.07% | 0.12 |
| **PatterniaPipe** | 1.714 | +0.07% | - | 0.40 |
| SwitchIndex | 1.868 | +9.12% | +9.04% | 0.13 |
| Sequential | 1.871 | +9.26% | +9.18% | 0.54 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.556 | fastest | -0.01% | 0.05 |
| **PatterniaPipe** | 1.557 | +0.01% | - | 0.09 |
| Sequential | 1.713 | +10.05% | +10.04% | 0.12 |
| SwitchIndex | 1.718 | +10.42% | +10.41% | 0.98 |

