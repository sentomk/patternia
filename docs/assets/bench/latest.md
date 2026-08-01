# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `0/8`
- Average Patternia gap vs fastest: `+25.15%`
- Largest Patternia gap: `VariantMixed` `+100.46%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.998 | SwitchIndex | 0.997 | +100.46% | 0.29 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.997 | SwitchIndex | 1.413 | +41.38% | 0.43 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.808 | StdVisit | 1.400 | +29.20% | 2.77 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.681 | Switch | 1.490 | +12.78% | 0.13 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.622 | Switch | 1.467 | +10.55% | 0.31 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.622 | StdVisit | 1.519 | +6.74% | 0.60 | watch |
| VariantAltHot | PatterniaPipe | 2/4 | 1.558 | StdVisit | 1.557 | +0.08% | 0.21 | close |
| VariantAltHotGuarded | PatterniaPipe | 2/4 | 1.714 | StdVisit | 1.713 | +0.01% | 0.11 | close |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.997 | fastest | -50.12% | 2.52 |
| Sequential | 1.164 | +16.72% | -41.77% | 0.34 |
| StdVisit | 1.993 | +99.89% | -0.28% | 0.24 |
| **PatterniaPipe** | 1.998 | +100.46% | - | 0.29 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.413 | fastest | -29.27% | 0.72 |
| Sequential | 1.431 | +1.30% | -28.35% | 0.60 |
| **PatterniaPipe** | 1.997 | +41.38% | - | 0.43 |
| StdVisit | 2.152 | +52.36% | +7.76% | 0.13 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.400 | fastest | -22.60% | 0.53 |
| **PatterniaPipe** | 1.808 | +29.20% | - | 2.77 |
| IfElse | 2.207 | +57.69% | +22.05% | 0.05 |
| Switch | 2.254 | +61.05% | +24.65% | 0.15 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.490 | fastest | -11.33% | 0.32 |
| **PatterniaPipe** | 1.681 | +12.78% | - | 0.13 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.467 | fastest | -9.54% | 1.48 |
| **PatterniaPipe** | 1.622 | +10.55% | - | 0.31 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.519 | fastest | -6.32% | 0.24 |
| Switch | 1.618 | +6.51% | -0.21% | 0.44 |
| **PatterniaPipe** | 1.622 | +6.74% | - | 0.60 |
| IfElse | 2.061 | +35.66% | +27.09% | 3.56 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.557 | fastest | -0.08% | 0.07 |
| **PatterniaPipe** | 1.558 | +0.08% | - | 0.21 |
| Sequential | 1.713 | +10.03% | +9.94% | 0.07 |
| SwitchIndex | 1.714 | +10.09% | +10.00% | 0.18 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.713 | fastest | -0.01% | 0.11 |
| **PatterniaPipe** | 1.714 | +0.01% | - | 0.11 |
| Sequential | 1.870 | +9.11% | +9.10% | 0.12 |
| SwitchIndex | 1.960 | +14.42% | +14.40% | 9.56 |

