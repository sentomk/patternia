# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `5`
- Patternia fastest: `0/5`
- Average Patternia gap vs fastest: `+14.50%`
- Largest Patternia gap: `LiteralMatch` `+55.29%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| LiteralMatch | PatterniaPipe | 3/3 | 2.540 | Switch | 1.636 | +55.29% | 0.41 | slow |
| ProtocolRouter | PatterniaPipe | 3/4 | 1.621 | StdVisit | 1.497 | +8.23% | 0.70 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.525 | Switch | 1.455 | +4.82% | 0.25 | close |
| CommandParser | PatterniaPipe | 2/4 | 1.665 | StdVisit | 1.608 | +3.55% | 1.19 | close |
| VariantMixed | PatterniaPipe | 2/4 | 2.003 | StdVisit | 1.991 | +0.61% | 2.32 | close |

---

## Per-Scenario Details
### LiteralMatch

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.636 | fastest | -35.60% | 0.14 |
| IfElse | 1.951 | +19.29% | -23.18% | 0.51 |
| **PatterniaPipe** | 2.540 | +55.29% | - | 0.41 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.497 | fastest | -7.60% | 1.41 |
| Switch | 1.595 | +6.49% | -1.61% | 1.03 |
| **PatterniaPipe** | 1.621 | +8.23% | - | 0.70 |
| IfElse | 1.890 | +26.23% | +16.63% | 0.52 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.455 | fastest | -4.60% | 1.47 |
| **PatterniaPipe** | 1.525 | +4.82% | - | 0.25 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.608 | fastest | -3.43% | 1.55 |
| **PatterniaPipe** | 1.665 | +3.55% | - | 1.19 |
| IfElse | 1.933 | +20.22% | +16.10% | 2.69 |
| Switch | 2.493 | +55.02% | +49.70% | 0.12 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.991 | fastest | -0.60% | 0.63 |
| **PatterniaPipe** | 2.003 | +0.61% | - | 2.32 |
| SwitchIndex | 2.141 | +7.55% | +6.90% | 0.14 |
| Sequential | 2.457 | +23.42% | +22.68% | 0.46 |

