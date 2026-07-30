# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `4`
- Patternia fastest: `1/4`
- Average Patternia gap vs fastest: `+11.73%`
- Largest Patternia gap: `PacketMixed` `+39.43%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 1.441 | Switch | 1.033 | +39.43% | 0.92 | slow |
| CommandParser | PatterniaPipe | 2/2 | 1.576 | Switch | 1.470 | +7.24% | 1.10 | watch |
| VariantMixed | PatterniaPipe | 2/2 | 1.262 | StdVisit | 1.259 | +0.24% | 2.93 | close |
| ProtocolRouter | PatterniaPipe | 1/2 | 1.261 | PatterniaPipe | 1.261 | +0.00% | 1.10 | fastest |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.033 | fastest | -28.28% | 0.59 |
| **PatterniaPipe** | 1.441 | +39.43% | - | 0.92 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.470 | fastest | -6.75% | 0.66 |
| **PatterniaPipe** | 1.576 | +7.24% | - | 1.10 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.259 | fastest | -0.24% | 2.67 |
| **PatterniaPipe** | 1.262 | +0.24% | - | 2.93 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.261 | fastest | - | 1.10 |
| IfElse | 1.864 | +47.79% | +47.79% | 0.91 |

