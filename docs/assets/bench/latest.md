# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `1/8`
- Average Patternia gap vs fastest: `+24.85%`
- Largest Patternia gap: `VariantMixed` `+100.18%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.999 | SwitchIndex | 0.998 | +100.18% | 0.28 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.986 | SwitchIndex | 1.413 | +40.52% | 0.85 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.773 | StdVisit | 1.390 | +27.54% | 0.14 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.683 | Switch | 1.496 | +12.52% | 0.27 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.622 | Switch | 1.459 | +11.20% | 0.22 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.623 | StdVisit | 1.521 | +6.72% | 0.82 | watch |
| VariantAltHot | PatterniaPipe | 2/4 | 1.558 | StdVisit | 1.556 | +0.09% | 0.25 | close |
| VariantAltHotGuarded | PatterniaPipe | 2/4 | 1.713 | StdVisit | 1.713 | +0.01% | 0.09 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.998 | fastest | -50.04% | 0.51 |
| Sequential | 1.162 | +16.34% | -41.88% | 0.14 |
| StdVisit | 1.992 | +99.55% | -0.31% | 0.31 |
| **PatterniaPipe** | 1.999 | +100.18% | - | 0.28 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.413 | fastest | -28.83% | 0.43 |
| Sequential | 1.427 | +0.97% | -28.14% | 0.26 |
| **PatterniaPipe** | 1.986 | +40.52% | - | 0.85 |
| StdVisit | 2.152 | +52.27% | +8.36% | 0.05 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.390 | fastest | -21.60% | 0.52 |
| **PatterniaPipe** | 1.773 | +27.54% | - | 0.14 |
| IfElse | 2.129 | +53.20% | +20.11% | 0.56 |
| Switch | 2.253 | +62.12% | +27.11% | 0.18 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.496 | fastest | -11.13% | 0.60 |
| **PatterniaPipe** | 1.683 | +12.52% | - | 0.27 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.459 | fastest | -10.07% | 1.47 |
| **PatterniaPipe** | 1.622 | +11.20% | - | 0.22 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.521 | fastest | -6.29% | 0.26 |
| Switch | 1.608 | +5.74% | -0.92% | 0.89 |
| **PatterniaPipe** | 1.623 | +6.72% | - | 0.82 |
| IfElse | 1.871 | +23.03% | +15.28% | 0.91 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.556 | fastest | -0.09% | 0.09 |
| **PatterniaPipe** | 1.558 | +0.09% | - | 0.25 |
| Sequential | 1.712 | +10.00% | +9.90% | 0.06 |
| SwitchIndex | 2.179 | +39.97% | +39.84% | 0.03 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.713 | fastest | -0.01% | 0.08 |
| **PatterniaPipe** | 1.713 | +0.01% | - | 0.09 |
| Sequential | 1.869 | +9.12% | +9.11% | 0.09 |
| SwitchIndex | 1.869 | +9.12% | +9.12% | 0.08 |

