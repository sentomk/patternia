# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `4`
- Patternia fastest: `2/4`
- Average Patternia gap vs fastest: `+2.84%`
- Largest Patternia gap: `PacketMixed` `+11.07%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 1.604 | Switch | 1.445 | +11.07% | 0.16 | watch |
| VariantMixed | PatterniaPipe | 2/2 | 1.997 | StdVisit | 1.991 | +0.31% | 0.29 | close |
| CommandParser | PatterniaPipe | 1/2 | 1.772 | PatterniaPipe | 1.772 | +0.00% | 0.17 | fastest |
| ProtocolRouter | PatterniaPipe | 1/2 | 1.618 | PatterniaPipe | 1.618 | +0.00% | 0.38 | fastest |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.445 | fastest | -9.97% | 1.37 |
| **PatterniaPipe** | 1.604 | +11.07% | - | 0.16 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.991 | fastest | -0.31% | 0.08 |
| **PatterniaPipe** | 1.997 | +0.31% | - | 0.29 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.772 | fastest | - | 0.17 |
| Switch | 2.247 | +26.79% | +26.79% | 0.20 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.618 | fastest | - | 0.38 |
| IfElse | 1.865 | +15.30% | +15.30% | 0.24 |

