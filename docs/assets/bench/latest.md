# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `0/8`
- Average Patternia gap vs fastest: `+24.90%`
- Largest Patternia gap: `VariantMixed` `+102.44%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.998 | SwitchIndex | 0.987 | +102.44% | 0.50 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.995 | Sequential | 1.445 | +38.11% | 0.05 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.776 | StdVisit | 1.389 | +27.84% | 0.39 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.690 | Switch | 1.503 | +12.43% | 0.55 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.621 | Switch | 1.450 | +11.77% | 0.26 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.632 | StdVisit | 1.533 | +6.44% | 1.19 | watch |
| VariantAltHotGuarded | PatterniaPipe | 2/4 | 1.713 | StdVisit | 1.712 | +0.08% | 0.26 | close |
| VariantAltHot | PatterniaPipe | 2/4 | 1.558 | StdVisit | 1.557 | +0.07% | 0.54 | close |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.987 | fastest | -50.60% | 5.98 |
| Sequential | 1.193 | +20.86% | -40.30% | 0.66 |
| StdVisit | 1.991 | +101.68% | -0.37% | 0.15 |
| **PatterniaPipe** | 1.998 | +102.44% | - | 0.50 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Sequential | 1.445 | fastest | -27.60% | 0.38 |
| SwitchIndex | 1.459 | +1.01% | -26.87% | 2.19 |
| **PatterniaPipe** | 1.995 | +38.11% | - | 0.05 |
| StdVisit | 2.154 | +49.08% | +7.94% | 0.34 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.389 | fastest | -21.78% | 0.65 |
| **PatterniaPipe** | 1.776 | +27.84% | - | 0.39 |
| IfElse | 2.189 | +57.58% | +23.27% | 1.56 |
| Switch | 2.253 | +62.17% | +26.86% | 0.52 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.503 | fastest | -11.06% | 1.29 |
| **PatterniaPipe** | 1.690 | +12.43% | - | 0.55 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.450 | fastest | -10.53% | 1.52 |
| **PatterniaPipe** | 1.621 | +11.77% | - | 0.26 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.533 | fastest | -6.05% | 0.52 |
| Switch | 1.615 | +5.31% | -1.06% | 1.17 |
| **PatterniaPipe** | 1.632 | +6.44% | - | 1.19 |
| IfElse | 1.867 | +21.75% | +14.39% | 0.21 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.712 | fastest | -0.08% | 0.03 |
| **PatterniaPipe** | 1.713 | +0.08% | - | 0.26 |
| Sequential | 1.869 | +9.19% | +9.10% | 0.27 |
| SwitchIndex | 2.267 | +32.44% | +32.33% | 6.91 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.557 | fastest | -0.07% | 0.16 |
| **PatterniaPipe** | 1.558 | +0.07% | - | 0.54 |
| Sequential | 1.713 | +10.03% | +9.95% | 0.29 |
| SwitchIndex | 1.763 | +13.19% | +13.11% | 8.14 |

