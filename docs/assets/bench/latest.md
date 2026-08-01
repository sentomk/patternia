# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `1/8`
- Average Patternia gap vs fastest: `+24.52%`
- Largest Patternia gap: `VariantMixed` `+98.92%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.996 | SwitchIndex | 1.003 | +98.92% | 0.28 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.994 | SwitchIndex | 1.420 | +40.40% | 0.23 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.777 | StdVisit | 1.396 | +27.33% | 1.44 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.682 | Switch | 1.496 | +12.43% | 0.24 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.622 | Switch | 1.463 | +10.85% | 0.28 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.613 | StdVisit | 1.519 | +6.17% | 0.24 | watch |
| VariantAltHot | PatterniaPipe | 2/4 | 1.558 | StdVisit | 1.557 | +0.09% | 0.24 | close |
| VariantAltHotGuarded | PatterniaPipe | 2/4 | 1.712 | StdVisit | 1.712 | +0.00% | 0.09 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.003 | fastest | -49.73% | 0.53 |
| Sequential | 1.167 | +16.29% | -41.54% | 0.33 |
| StdVisit | 1.991 | +98.43% | -0.24% | 0.44 |
| **PatterniaPipe** | 1.996 | +98.92% | - | 0.28 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.420 | fastest | -28.77% | 1.41 |
| Sequential | 1.427 | +0.44% | -28.46% | 0.34 |
| **PatterniaPipe** | 1.994 | +40.40% | - | 0.23 |
| StdVisit | 2.152 | +51.48% | +7.89% | 0.11 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.396 | fastest | -21.46% | 0.51 |
| **PatterniaPipe** | 1.777 | +27.33% | - | 1.44 |
| IfElse | 2.204 | +57.93% | +24.03% | 0.41 |
| Switch | 2.253 | +61.39% | +26.75% | 0.19 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.496 | fastest | -11.05% | 0.74 |
| **PatterniaPipe** | 1.682 | +12.43% | - | 0.24 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.463 | fastest | -9.79% | 1.15 |
| **PatterniaPipe** | 1.622 | +10.85% | - | 0.28 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.519 | fastest | -5.81% | 0.57 |
| Switch | 1.590 | +4.66% | -1.42% | 0.54 |
| **PatterniaPipe** | 1.613 | +6.17% | - | 0.24 |
| IfElse | 1.966 | +29.43% | +21.91% | 5.74 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.557 | fastest | -0.09% | 0.09 |
| **PatterniaPipe** | 1.558 | +0.09% | - | 0.24 |
| SwitchIndex | 1.713 | +10.04% | +9.94% | 0.11 |
| Sequential | 1.716 | +10.24% | +10.14% | 0.50 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.712 | fastest | -0.00% | 0.12 |
| **PatterniaPipe** | 1.712 | +0.00% | - | 0.09 |
| Sequential | 1.868 | +9.10% | +9.09% | 0.11 |
| SwitchIndex | 1.903 | +11.17% | +11.17% | 8.09 |

