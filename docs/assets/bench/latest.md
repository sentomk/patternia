# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `5`
- Patternia fastest: `0/5`
- Average Patternia gap vs fastest: `+18.74%`
- Largest Patternia gap: `LiteralMatch` `+52.52%` vs `IfElse`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| LiteralMatch | PatterniaPipe | 3/3 | 2.502 | IfElse | 1.641 | +52.52% | 0.51 | slow |
| VariantMixed | PatterniaPipe | 3/4 | 2.299 | StdVisit | 1.985 | +15.80% | 0.40 | watch |
| PacketMixed | PatterniaPipe | 2/2 | 1.626 | Switch | 1.456 | +11.66% | 0.48 | watch |
| ProtocolRouter | PatterniaPipe | 2/4 | 1.624 | StdVisit | 1.517 | +7.04% | 0.68 | watch |
| CommandParser | PatterniaPipe | 2/4 | 1.772 | IfElse | 1.661 | +6.66% | 0.10 | watch |

---

## Per-Scenario Details
### LiteralMatch

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| IfElse | 1.641 | fastest | -34.44% | 0.67 |
| Switch | 2.007 | +22.31% | -19.81% | 21.38 |
| **PatterniaPipe** | 2.502 | +52.52% | - | 0.51 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.985 | fastest | -13.64% | 0.09 |
| Sequential | 2.147 | +8.14% | -6.61% | 0.72 |
| **PatterniaPipe** | 2.299 | +15.80% | - | 0.40 |
| SwitchIndex | 2.456 | +23.70% | +6.82% | 0.59 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.456 | fastest | -10.44% | 1.58 |
| **PatterniaPipe** | 1.626 | +11.66% | - | 0.48 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.517 | fastest | -6.58% | 0.46 |
| **PatterniaPipe** | 1.624 | +7.04% | - | 0.68 |
| Switch | 1.723 | +13.56% | +6.10% | 0.56 |
| IfElse | 1.798 | +18.51% | +10.72% | 0.31 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| IfElse | 1.661 | fastest | -6.25% | 1.05 |
| **PatterniaPipe** | 1.772 | +6.66% | - | 0.10 |
| Switch | 1.929 | +16.10% | +8.85% | 0.50 |
| StdVisit | 1.965 | +18.30% | +10.91% | 0.56 |

