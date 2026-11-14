# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `4/8`
- Average Patternia gap vs fastest: `+21.02%`
- Largest Patternia gap: `VariantMixedGuarded` `+136.57%` vs `Sequential`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixedGuarded | PatterniaPipe | 4/4 | 1.472 | Sequential | 0.622 | +136.57% | 1.55 | slow |
| ProtocolRouter | PatterniaPipe | 3/4 | 0.924 | Switch | 0.774 | +19.44% | 1.33 | watch |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 0.912 | Switch | 0.846 | +7.78% | 3.24 | watch |
| CommandParser | PatterniaPipe | 2/4 | 0.827 | Switch | 0.793 | +4.34% | 1.66 | close |
| PacketMixed | PatterniaPipe | 1/2 | 0.841 | PatterniaPipe | 0.841 | +0.00% | 3.65 | fastest |
| VariantAltHot | PatterniaPipe | 1/4 | 1.104 | PatterniaPipe | 1.104 | +0.00% | 2.23 | fastest |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.223 | PatterniaPipe | 1.223 | +0.00% | 2.55 | fastest |
| VariantMixed | PatterniaPipe | 1/4 | 0.552 | PatterniaPipe | 0.552 | +0.00% | 2.02 | fastest |

---

## Per-Scenario Details
### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Sequential | 0.622 | fastest | -57.73% | 0.94 |
| SwitchIndex | 0.734 | +17.89% | -50.17% | 1.06 |
| StdVisit | 1.467 | +135.71% | -0.37% | 1.26 |
| **PatterniaPipe** | 1.472 | +136.57% | - | 1.55 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 0.774 | fastest | -16.28% | 2.20 |
| StdVisit | 0.879 | +13.59% | -4.90% | 0.64 |
| **PatterniaPipe** | 0.924 | +19.44% | - | 1.33 |
| IfElse | 0.947 | +22.38% | +2.46% | 3.54 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 0.846 | fastest | -7.22% | 2.27 |
| **PatterniaPipe** | 0.912 | +7.78% | - | 3.24 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 0.793 | fastest | -4.16% | 3.71 |
| **PatterniaPipe** | 0.827 | +4.34% | - | 1.66 |
| IfElse | 0.903 | +13.82% | +9.09% | 2.62 |
| StdVisit | 0.964 | +21.62% | +16.56% | 2.41 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 0.841 | fastest | - | 3.65 |
| Switch | 0.961 | +14.34% | +14.34% | 2.98 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.104 | fastest | - | 2.23 |
| StdVisit | 1.339 | +21.29% | +21.29% | 2.13 |
| Sequential | 1.629 | +47.50% | +47.50% | 2.46 |
| SwitchIndex | 1.641 | +48.58% | +48.58% | 2.48 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.223 | fastest | - | 2.55 |
| StdVisit | 1.447 | +18.34% | +18.34% | 5.31 |
| Sequential | 1.497 | +22.41% | +22.41% | 3.55 |
| SwitchIndex | 1.549 | +26.70% | +26.70% | 1.77 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 0.552 | fastest | - | 2.02 |
| SwitchIndex | 0.615 | +11.36% | +11.36% | 2.37 |
| StdVisit | 0.649 | +17.43% | +17.43% | 1.34 |
| Sequential | 1.483 | +168.43% | +168.43% | 1.50 |

