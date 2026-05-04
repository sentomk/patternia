# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `4`
- Patternia fastest: `2/4`
- Average Patternia gap vs fastest: `+3.02%`
- Largest Patternia gap: `PacketMixed` `+11.65%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 1.625 | Switch | 1.455 | +11.65% | 0.22 | watch |
| VariantMixed | PatterniaPipe | 2/2 | 2.017 | StdVisit | 2.008 | +0.43% | 0.94 | close |
| CommandParser | PatterniaPipe | 1/2 | 1.773 | PatterniaPipe | 1.773 | +0.00% | 0.22 | fastest |
| ProtocolRouter | PatterniaPipe | 1/2 | 1.624 | PatterniaPipe | 1.624 | +0.00% | 0.36 | fastest |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.455 | fastest | -10.44% | 1.73 |
| **PatterniaPipe** | 1.625 | +11.65% | - | 0.22 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 2.008 | fastest | -0.43% | 1.05 |
| **PatterniaPipe** | 2.017 | +0.43% | - | 0.94 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.773 | fastest | - | 0.22 |
| Switch | 2.255 | +27.22% | +27.22% | 0.38 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.624 | fastest | - | 0.36 |
| IfElse | 1.866 | +14.90% | +14.90% | 0.41 |

