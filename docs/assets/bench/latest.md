# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `4`
- Patternia fastest: `2/4`
- Average Patternia gap vs fastest: `+3.07%`
- Largest Patternia gap: `PacketMixed` `+11.99%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 1.625 | Switch | 1.451 | +11.99% | 0.38 | watch |
| VariantMixed | PatterniaPipe | 2/2 | 1.997 | StdVisit | 1.991 | +0.31% | 0.22 | close |
| CommandParser | PatterniaPipe | 1/2 | 1.790 | PatterniaPipe | 1.790 | +0.00% | 1.90 | fastest |
| ProtocolRouter | PatterniaPipe | 1/2 | 1.628 | PatterniaPipe | 1.628 | +0.00% | 1.55 | fastest |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.451 | fastest | -10.70% | 1.90 |
| **PatterniaPipe** | 1.625 | +11.99% | - | 0.38 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.991 | fastest | -0.31% | 0.07 |
| **PatterniaPipe** | 1.997 | +0.31% | - | 0.22 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.790 | fastest | - | 1.90 |
| Switch | 2.258 | +26.17% | +26.17% | 0.62 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.628 | fastest | - | 1.55 |
| IfElse | 1.866 | +14.61% | +14.61% | 0.22 |

