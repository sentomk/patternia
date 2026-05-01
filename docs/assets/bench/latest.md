# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `13`
- Patternia fastest: `4/13`
- Average Patternia gap vs fastest: `+37.17%`
- Largest Patternia gap: `LiteralMatchRDense` `+79.48%` vs `IfElse`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| LiteralMatchRDense | PatterniaPipe | 3/3 | 2.849 | IfElse | 1.587 | +79.48% | 0.22 | slow |
| LiteralMatch64 | PatterniaPipe | 3/3 | 2.715 | Switch | 1.615 | +68.12% | 0.24 | slow |
| LiteralMatch16 | PatterniaPipe | 3/3 | 2.420 | Switch | 1.470 | +64.61% | 0.04 | slow |
| LiteralMatch | PatterniaPipe | 3/3 | 2.460 | Switch | 1.501 | +63.91% | 0.12 | slow |
| VariantMixed | PatterniaPipe | 4/4 | 1.457 | SwitchIndex | 0.936 | +55.66% | 0.28 | slow |
| LiteralMatch32 | PatterniaPipe | 3/3 | 2.687 | IfElse | 1.820 | +47.63% | 0.03 | slow |
| PacketMixed | PatterniaPipe | 2/2 | 2.055 | Switch | 1.454 | +41.28% | 0.87 | slow |
| CommandParser | PatterniaPipe | 4/4 | 1.981 | StdVisit | 1.579 | +25.41% | 0.86 | slow |
| LiteralMatch128On | PatterniaPipe | 1/1 | 2.748 | PatterniaPipe | 2.748 | +0.00% | 2.77 | fastest |
| LiteralMatch128OnMacro | PatterniaPipe | 1/1 | 5.144 | PatterniaPipe | 5.144 | +0.00% | 3.37 | fastest |
| LiteralMatch128StaticCases | PatterniaPipe | 1/1 | 5.495 | PatterniaPipe | 5.495 | +0.00% | 2.90 | fastest |
| ProtocolRouter | PatterniaPipe | 1/4 | 1.695 | PatterniaPipe | 1.695 | +0.00% | 1.46 | fastest |
| LiteralMatch128 | - | - | - | Switch | 1.633 | - | - | missing |

---

## Per-Scenario Details
### LiteralMatchRDense

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| IfElse | 1.587 | fastest | -44.28% | 0.58 |
| Switch | 1.625 | +2.37% | -42.96% | 0.33 |
| **PatterniaPipe** | 2.849 | +79.48% | - | 0.22 |

### LiteralMatch64

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.615 | fastest | -40.52% | 0.72 |
| IfElse | 1.647 | +2.01% | -39.32% | 2.27 |
| **PatterniaPipe** | 2.715 | +68.12% | - | 0.24 |

### LiteralMatch16

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.470 | fastest | -39.25% | 0.09 |
| IfElse | 1.563 | +6.34% | -35.40% | 0.51 |
| **PatterniaPipe** | 2.420 | +64.61% | - | 0.04 |

### LiteralMatch

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.501 | fastest | -38.99% | 0.12 |
| IfElse | 1.595 | +6.32% | -35.14% | 0.45 |
| **PatterniaPipe** | 2.460 | +63.91% | - | 0.12 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| SwitchIndex | 0.936 | fastest | -35.76% | 0.17 |
| Sequential | 1.051 | +12.25% | -27.88% | 0.50 |
| StdVisit | 1.455 | +55.44% | -0.14% | 0.07 |
| **PatterniaPipe** | 1.457 | +55.66% | - | 0.28 |

### LiteralMatch32

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| IfElse | 1.820 | fastest | -32.26% | 2.06 |
| Switch | 1.861 | +2.22% | -30.76% | 3.09 |
| **PatterniaPipe** | 2.687 | +47.63% | - | 0.03 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.454 | fastest | -29.22% | 0.71 |
| **PatterniaPipe** | 2.055 | +41.28% | - | 0.87 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.579 | fastest | -20.26% | 1.31 |
| IfElse | 1.908 | +20.78% | -3.69% | 0.67 |
| Switch | 1.923 | +21.74% | -2.92% | 0.63 |
| **PatterniaPipe** | 1.981 | +25.41% | - | 0.86 |

### LiteralMatch128On

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 2.748 | fastest | - | 2.77 |

### LiteralMatch128OnMacro

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 5.144 | fastest | - | 3.37 |

### LiteralMatch128StaticCases

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 5.495 | fastest | - | 2.90 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.695 | fastest | - | 1.46 |
| StdVisit | 1.752 | +3.38% | +3.38% | 1.11 |
| Switch | 1.998 | +17.91% | +17.91% | 0.86 |
| IfElse | 2.575 | +51.95% | +51.95% | 0.45 |

### LiteralMatch128

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.633 | fastest | - | 1.65 |

