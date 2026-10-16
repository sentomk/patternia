# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `2/8`
- Average Patternia gap vs fastest: `+24.94%`
- Largest Patternia gap: `VariantMixed` `+99.90%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.996 | SwitchIndex | 0.998 | +99.90% | 0.18 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 2.001 | SwitchIndex | 1.420 | +40.90% | 0.50 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.785 | StdVisit | 1.393 | +28.18% | 1.26 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.625 | Switch | 1.439 | +12.96% | 0.77 | watch |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.666 | Switch | 1.497 | +11.28% | 0.27 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.622 | StdVisit | 1.525 | +6.33% | 0.90 | watch |
| VariantAltHot | PatterniaPipe | 1/4 | 1.558 | PatterniaPipe | 1.558 | +0.00% | 0.32 | fastest |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.716 | PatterniaPipe | 1.716 | +0.00% | 0.58 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.998 | fastest | -49.97% | 4.84 |
| Sequential | 1.169 | +17.08% | -41.43% | 0.49 |
| StdVisit | 1.993 | +99.58% | -0.16% | 0.21 |
| **PatterniaPipe** | 1.996 | +99.90% | - | 0.18 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 1.420 | fastest | -29.03% | 0.97 |
| Sequential | 1.432 | +0.88% | -28.41% | 0.74 |
| **PatterniaPipe** | 2.001 | +40.90% | - | 0.50 |
| StdVisit | 2.154 | +51.68% | +7.65% | 0.31 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.393 | fastest | -21.99% | 0.44 |
| **PatterniaPipe** | 1.785 | +28.18% | - | 1.26 |
| IfElse | 2.194 | +57.56% | +22.92% | 1.50 |
| Switch | 2.253 | +61.76% | +26.19% | 0.45 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.439 | fastest | -11.47% | 1.80 |
| **PatterniaPipe** | 1.625 | +12.96% | - | 0.77 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.497 | fastest | -10.14% | 0.21 |
| **PatterniaPipe** | 1.666 | +11.28% | - | 0.27 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.525 | fastest | -5.96% | 0.87 |
| **PatterniaPipe** | 1.622 | +6.33% | - | 0.90 |
| Switch | 1.625 | +6.51% | +0.17% | 1.13 |
| IfElse | 1.891 | +24.00% | +16.61% | 1.81 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.558 | fastest | - | 0.32 |
| StdVisit | 1.558 | +0.05% | +0.05% | 0.33 |
| Sequential | 1.715 | +10.10% | +10.10% | 0.37 |
| SwitchIndex | 1.717 | +10.20% | +10.20% | 0.57 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.716 | fastest | - | 0.58 |
| StdVisit | 1.717 | +0.05% | +0.05% | 0.83 |
| SwitchIndex | 1.869 | +8.86% | +8.86% | 0.09 |
| Sequential | 1.870 | +8.95% | +8.95% | 0.25 |

