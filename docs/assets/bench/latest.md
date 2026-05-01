# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `4`
- Patternia fastest: `1/4`
- Average Patternia gap vs fastest: `+9.80%`
- Largest Patternia gap: `PacketMixed` `+35.69%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 2.080 | Switch | 1.533 | +35.69% | 0.97 | slow |
| CommandParser | PatterniaPipe | 2/2 | 1.983 | Switch | 1.917 | +3.44% | 0.64 | close |
| VariantMixed | PatterniaPipe | 2/2 | 1.455 | StdVisit | 1.455 | +0.05% | 0.13 | close |
| ProtocolRouter | PatterniaPipe | 1/2 | 1.753 | PatterniaPipe | 1.753 | +0.00% | 4.12 | fastest |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.533 | fastest | -26.30% | 4.87 |
| **PatterniaPipe** | 2.080 | +35.69% | - | 0.97 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.917 | fastest | -3.33% | 0.21 |
| **PatterniaPipe** | 1.983 | +3.44% | - | 0.64 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.455 | fastest | -0.05% | 0.16 |
| **PatterniaPipe** | 1.455 | +0.05% | - | 0.13 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.753 | fastest | - | 4.12 |
| IfElse | 2.564 | +46.25% | +46.25% | 0.67 |

