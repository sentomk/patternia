# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `4`
- Patternia fastest: `3/4`
- Average Patternia gap vs fastest: `+0.67%`
- Largest Patternia gap: `PacketMixed` `+2.68%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 1.517 | Switch | 1.477 | +2.68% | 4.99 | close |
| CommandParser | PatterniaPipe | 1/2 | 1.662 | PatterniaPipe | 1.662 | +0.00% | 0.39 | fastest |
| ProtocolRouter | PatterniaPipe | 1/2 | 1.620 | PatterniaPipe | 1.620 | +0.00% | 0.57 | fastest |
| VariantMixed | PatterniaPipe | 1/2 | 0.936 | PatterniaPipe | 0.936 | +0.00% | 1.93 | fastest |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.477 | fastest | -2.61% | 1.50 |
| **PatterniaPipe** | 1.517 | +2.68% | - | 4.99 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.662 | fastest | - | 0.39 |
| Switch | 2.496 | +50.19% | +50.19% | 0.54 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.620 | fastest | - | 0.57 |
| IfElse | 1.886 | +16.39% | +16.39% | 0.24 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 0.936 | fastest | - | 1.93 |
| StdVisit | 1.992 | +112.91% | +112.91% | 0.49 |

