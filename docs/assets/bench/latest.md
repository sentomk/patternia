# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `1/8`
- Average Patternia gap vs fastest: `+24.04%`
- Largest Patternia gap: `VariantMixed` `+97.98%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.998 | SwitchIndex | 1.009 | +97.98% | 0.40 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.915 | SwitchIndex | 1.413 | +35.50% | 2.13 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.774 | StdVisit | 1.397 | +26.97% | 0.21 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.621 | Switch | 1.441 | +12.49% | 0.25 | watch |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.682 | Switch | 1.495 | +12.49% | 0.20 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.627 | StdVisit | 1.523 | +6.83% | 0.29 | watch |
| VariantAltHotGuarded | PatterniaPipe | 2/4 | 1.716 | StdVisit | 1.714 | +0.08% | 0.57 | close |
| VariantAltHot | PatterniaPipe | 1/4 | 1.559 | PatterniaPipe | 1.559 | +0.00% | 0.47 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.009 | fastest | -49.49% | 0.59 |
| Sequential | 1.162 | +15.15% | -41.84% | 0.15 |
| StdVisit | 1.990 | +97.26% | -0.36% | 0.11 |
| **PatterniaPipe** | 1.998 | +97.98% | - | 0.40 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.413 | fastest | -26.20% | 0.83 |
| Sequential | 1.430 | +1.19% | -25.32% | 0.31 |
| **PatterniaPipe** | 1.915 | +35.50% | - | 2.13 |
| StdVisit | 2.155 | +52.47% | +12.52% | 0.35 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.397 | fastest | -21.24% | 0.49 |
| **PatterniaPipe** | 1.774 | +26.97% | - | 0.21 |
| IfElse | 2.128 | +52.36% | +20.00% | 0.44 |
| Switch | 2.254 | +61.35% | +27.08% | 0.22 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.441 | fastest | -11.11% | 1.69 |
| **PatterniaPipe** | 1.621 | +12.49% | - | 0.25 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.495 | fastest | -11.10% | 0.44 |
| **PatterniaPipe** | 1.682 | +12.49% | - | 0.20 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.523 | fastest | -6.39% | 0.37 |
| Switch | 1.608 | +5.59% | -1.16% | 0.48 |
| **PatterniaPipe** | 1.627 | +6.83% | - | 0.29 |
| IfElse | 1.936 | +27.12% | +19.00% | 5.89 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.714 | fastest | -0.08% | 0.48 |
| **PatterniaPipe** | 1.716 | +0.08% | - | 0.57 |
| Sequential | 1.871 | +9.12% | +9.03% | 0.49 |
| SwitchIndex | 2.338 | +36.38% | +36.27% | 0.50 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.559 | fastest | - | 0.47 |
| StdVisit | 1.560 | +0.10% | +0.10% | 0.77 |
| Sequential | 1.713 | +9.87% | +9.87% | 0.14 |
| SwitchIndex | 1.714 | +9.95% | +9.95% | 0.48 |

