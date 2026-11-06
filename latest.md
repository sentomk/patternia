# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `1/8`
- Average Patternia gap vs fastest: `+25.35%`
- Largest Patternia gap: `VariantMixed` `+100.28%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.999 | SwitchIndex | 0.998 | +100.28% | 0.29 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 1.998 | SwitchIndex | 1.409 | +41.82% | 0.12 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.779 | StdVisit | 1.387 | +28.29% | 0.48 | slow |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.687 | Switch | 1.499 | +12.57% | 0.22 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.622 | Switch | 1.443 | +12.42% | 0.22 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.627 | StdVisit | 1.518 | +7.20% | 0.18 | watch |
| VariantAltHotGuarded | PatterniaPipe | 2/4 | 1.717 | StdVisit | 1.714 | +0.18% | 0.54 | close |
| VariantAltHot | PatterniaPipe | 1/4 | 1.559 | PatterniaPipe | 1.559 | +0.00% | 0.09 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.998 | fastest | -50.07% | 3.26 |
| Sequential | 1.167 | +16.90% | -41.63% | 0.18 |
| StdVisit | 1.993 | +99.72% | -0.28% | 0.17 |
| **PatterniaPipe** | 1.999 | +100.28% | - | 0.29 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.409 | fastest | -29.49% | 0.40 |
| Sequential | 1.435 | +1.84% | -28.19% | 0.69 |
| **PatterniaPipe** | 1.998 | +41.82% | - | 0.12 |
| StdVisit | 2.155 | +52.93% | +7.83% | 0.43 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.387 | fastest | -22.05% | 0.40 |
| **PatterniaPipe** | 1.779 | +28.29% | - | 0.48 |
| IfElse | 2.143 | +54.60% | +20.51% | 1.90 |
| Switch | 2.261 | +63.05% | +27.10% | 1.07 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.499 | fastest | -11.17% | 0.23 |
| **PatterniaPipe** | 1.687 | +12.57% | - | 0.22 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.443 | fastest | -11.05% | 1.47 |
| **PatterniaPipe** | 1.622 | +12.42% | - | 0.22 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.518 | fastest | -6.71% | 0.24 |
| Switch | 1.590 | +4.71% | -2.32% | 0.43 |
| **PatterniaPipe** | 1.627 | +7.20% | - | 0.18 |
| IfElse | 1.867 | +23.01% | +14.75% | 0.20 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.714 | fastest | -0.18% | 0.08 |
| **PatterniaPipe** | 1.717 | +0.18% | - | 0.54 |
| Sequential | 1.870 | +9.13% | +8.94% | 0.10 |
| SwitchIndex | 1.871 | +9.15% | +8.96% | 0.11 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.559 | fastest | - | 0.09 |
| StdVisit | 1.565 | +0.38% | +0.38% | 0.80 |
| SwitchIndex | 1.716 | +10.11% | +10.11% | 0.56 |
| Sequential | 1.994 | +27.93% | +27.93% | 11.73 |

