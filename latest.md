# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `8`
- Patternia fastest: `2/8`
- Average Patternia gap vs fastest: `+25.86%`
- Largest Patternia gap: `VariantMixed` `+106.43%` vs `SwitchIndex`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| VariantMixed | PatterniaPipe | 4/4 | 1.998 | SwitchIndex | 0.968 | +106.43% | 0.30 | slow |
| VariantMixedGuarded | PatterniaPipe | 3/4 | 2.005 | Sequential | 1.430 | +40.19% | 1.15 | slow |
| CommandParser | PatterniaPipe | 2/4 | 1.780 | StdVisit | 1.391 | +27.93% | 0.68 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 1.624 | Switch | 1.435 | +13.21% | 0.30 | watch |
| PacketMixedHeavyBind | PatterniaPipe | 2/2 | 1.684 | Switch | 1.492 | +12.87% | 0.43 | watch |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.619 | StdVisit | 1.524 | +6.28% | 0.47 | watch |
| VariantAltHot | PatterniaPipe | 1/4 | 1.559 | PatterniaPipe | 1.559 | +0.00% | 0.37 | fastest |
| VariantAltHotGuarded | PatterniaPipe | 1/4 | 1.713 | PatterniaPipe | 1.713 | +0.00% | 0.10 | fastest |

---

## Per-Scenario Details
### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.968 | fastest | -51.56% | 4.72 |
| Sequential | 1.168 | +20.66% | -41.55% | 0.15 |
| StdVisit | 1.997 | +106.38% | -0.03% | 0.44 |
| **PatterniaPipe** | 1.998 | +106.43% | - | 0.30 |

### VariantMixedGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Sequential | 1.430 | fastest | -28.67% | 0.66 |
| SwitchIndex | 1.441 | +0.79% | -28.11% | 2.16 |
| **PatterniaPipe** | 2.005 | +40.19% | - | 1.15 |
| StdVisit | 2.153 | +50.52% | +7.36% | 0.27 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.391 | fastest | -21.83% | 0.85 |
| **PatterniaPipe** | 1.780 | +27.93% | - | 0.68 |
| IfElse | 2.190 | +57.37% | +23.01% | 1.49 |
| Switch | 2.253 | +61.92% | +26.57% | 0.66 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.435 | fastest | -11.67% | 1.55 |
| **PatterniaPipe** | 1.624 | +13.21% | - | 0.30 |

### PacketMixedHeavyBind

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.492 | fastest | -11.40% | 0.15 |
| **PatterniaPipe** | 1.684 | +12.87% | - | 0.43 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.524 | fastest | -5.91% | 0.62 |
| Switch | 1.618 | +6.22% | -0.06% | 0.87 |
| **PatterniaPipe** | 1.619 | +6.28% | - | 0.47 |
| IfElse | 1.955 | +28.31% | +20.73% | 5.90 |

### VariantAltHot

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.559 | fastest | - | 0.37 |
| StdVisit | 1.564 | +0.26% | +0.26% | 0.67 |
| Sequential | 1.713 | +9.84% | +9.84% | 0.11 |
| SwitchIndex | 1.717 | +10.09% | +10.09% | 0.56 |

### VariantAltHotGuarded

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.713 | fastest | - | 0.10 |
| StdVisit | 1.716 | +0.14% | +0.14% | 0.44 |
| SwitchIndex | 1.869 | +9.11% | +9.11% | 0.21 |
| Sequential | 1.877 | +9.56% | +9.56% | 0.64 |

