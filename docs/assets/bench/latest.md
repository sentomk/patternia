# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `4`
- Patternia fastest: `2/4`
- Average Patternia gap vs fastest: `+8.26%`
- Largest Patternia gap: `PacketMixed` `+20.56%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 1.817 | Switch | 1.507 | +20.56% | 0.12 | slow |
| CommandParser | PatterniaPipe | 2/2 | 2.025 | Switch | 1.800 | +12.49% | 1.82 | watch |
| ProtocolRouter | PatterniaPipe | 1/2 | 1.694 | PatterniaPipe | 1.694 | +0.00% | 0.49 | fastest |
| VariantMixed | PatterniaPipe | 1/2 | 2.126 | PatterniaPipe | 2.126 | +0.00% | 0.07 | fastest |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.507 | fastest | -17.05% | 4.24 |
| **PatterniaPipe** | 1.817 | +20.56% | - | 0.12 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.800 | fastest | -11.10% | 0.62 |
| **PatterniaPipe** | 2.025 | +12.49% | - | 1.82 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.694 | fastest | - | 0.49 |
| IfElse | 2.166 | +27.83% | +27.83% | 1.86 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 2.126 | fastest | - | 0.07 |
| StdVisit | 2.890 | +35.93% | +35.93% | 16.31 |

