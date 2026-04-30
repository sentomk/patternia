# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `13`
- Patternia fastest: `3/13`
- Average Patternia gap vs fastest: `+82.14%`
- Largest Patternia gap: `LiteralMatch64` `+443.92%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| LiteralMatch64 | PatterniaPipe | 3/3 | 8.802 | Switch | 1.618 | +443.92% | 0.24 | slow |
| LiteralMatch32 | PatterniaPipe | 3/3 | 5.180 | IfElse | 1.675 | +209.35% | 0.11 | slow |
| LiteralMatch16 | PatterniaPipe | 3/3 | 3.620 | Switch | 1.596 | +126.86% | 0.57 | slow |
| LiteralMatchRDense | PatterniaPipe | 3/3 | 3.530 | IfElse | 1.673 | +110.96% | 0.43 | slow |
| LiteralMatch | PatterniaPipe | 3/3 | 2.503 | IfElse | 1.639 | +52.68% | 0.46 | slow |
| VariantMixed | PatterniaPipe | 3/4 | 2.300 | StdVisit | 1.987 | +15.75% | 0.69 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.624 | Switch | 1.450 | +12.03% | 0.25 | watch |
| CommandParser | PatterniaPipe | 2/4 | 1.775 | IfElse | 1.657 | +7.12% | 0.48 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.624 | StdVisit | 1.518 | +6.98% | 0.38 | watch |
| LiteralMatch128On | PatterniaPipe | 1/1 | 10.768 | PatterniaPipe | 10.768 | +0.00% | 0.50 | fastest |
| LiteralMatch128OnMacro | PatterniaPipe | 1/1 | 10.521 | PatterniaPipe | 10.521 | +0.00% | 0.46 | fastest |
| LiteralMatch128StaticCases | PatterniaPipe | 1/1 | 10.453 | PatterniaPipe | 10.453 | +0.00% | 1.96 | fastest |
| LiteralMatch128 | - | - | - | Switch | 1.584 | - | - | missing |

---

## Per-Scenario Details
### LiteralMatch64

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.618 | fastest | -81.62% | 0.43 |
| IfElse | 1.929 | +19.20% | -78.08% | 0.07 |
| **PatterniaPipe** | 8.802 | +443.92% | - | 0.24 |

### LiteralMatch32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| IfElse | 1.675 | fastest | -67.67% | 0.40 |
| Switch | 1.676 | +0.05% | -67.66% | 0.43 |
| **PatterniaPipe** | 5.180 | +209.35% | - | 0.11 |

### LiteralMatch16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.596 | fastest | -55.92% | 0.58 |
| IfElse | 1.907 | +19.52% | -47.32% | 0.51 |
| **PatterniaPipe** | 3.620 | +126.86% | - | 0.57 |

### LiteralMatchRDense

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| IfElse | 1.673 | fastest | -52.60% | 0.40 |
| Switch | 1.951 | +16.57% | -44.74% | 0.10 |
| **PatterniaPipe** | 3.530 | +110.96% | - | 0.43 |

### LiteralMatch

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| IfElse | 1.639 | fastest | -34.51% | 0.62 |
| Switch | 1.641 | +0.09% | -34.45% | 0.72 |
| **PatterniaPipe** | 2.503 | +52.68% | - | 0.46 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.987 | fastest | -13.61% | 0.37 |
| Sequential | 2.143 | +7.88% | -6.80% | 0.20 |
| **PatterniaPipe** | 2.300 | +15.75% | - | 0.69 |
| SwitchIndex | 3.385 | +70.40% | +47.21% | 0.07 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.450 | fastest | -10.74% | 1.40 |
| **PatterniaPipe** | 1.624 | +12.03% | - | 0.25 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| IfElse | 1.657 | fastest | -6.65% | 0.22 |
| **PatterniaPipe** | 1.775 | +7.12% | - | 0.48 |
| Switch | 1.930 | +16.48% | +8.74% | 0.33 |
| StdVisit | 1.966 | +18.69% | +10.80% | 0.56 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.518 | fastest | -6.53% | 0.64 |
| **PatterniaPipe** | 1.624 | +6.98% | - | 0.38 |
| Switch | 1.725 | +13.61% | +6.19% | 0.50 |
| IfElse | 1.801 | +18.63% | +10.88% | 0.71 |

### LiteralMatch128On

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 10.768 | fastest | - | 0.50 |

### LiteralMatch128OnMacro

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 10.521 | fastest | - | 0.46 |

### LiteralMatch128StaticCases

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 10.453 | fastest | - | 1.96 |

### LiteralMatch128

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.584 | fastest | - | 0.09 |

