# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `4`
- Patternia fastest: `2/4`
- Average Patternia gap vs fastest: `+2.59%`
- Largest Patternia gap: `PacketMixed` `+10.18%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 1.625 | Switch | 1.475 | +10.18% | 0.31 | watch |
| VariantMixed | PatterniaPipe | 2/2 | 1.995 | StdVisit | 1.992 | +0.17% | 0.14 | close |
| CommandParser | PatterniaPipe | 1/2 | 1.772 | PatterniaPipe | 1.772 | +0.00% | 0.07 | fastest |
| ProtocolRouter | PatterniaPipe | 1/2 | 1.622 | PatterniaPipe | 1.622 | +0.00% | 0.19 | fastest |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.475 | fastest | -9.24% | 1.64 |
| **PatterniaPipe** | 1.625 | +10.18% | - | 0.31 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.992 | fastest | -0.17% | 0.22 |
| **PatterniaPipe** | 1.995 | +0.17% | - | 0.14 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.772 | fastest | - | 0.07 |
| Switch | 2.255 | +27.29% | +27.29% | 0.23 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.622 | fastest | - | 0.19 |
| IfElse | 2.004 | +23.54% | +23.54% | 5.52 |

