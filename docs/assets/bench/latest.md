# Single Benchmark Report

- Source: `/home/runner/work/patternia/patternia/bench_results/ptn_bench.json`

## Summary

| Scenario | Fastest | Mean (ns) | Patternia vs fastest |
|---|---:|---:|---:|
| CommandParser | Switch | 1.343 | 1.321x |
| LiteralMatch | Switch | 0.901 | 1.997x |
| PacketMixed | PatterniaPipe | 1.405 | 1.000x |
| ProtocolRouter | StdVisit | 1.492 | 1.120x |
| VariantMixed | PatterniaPipe | 1.069 | 1.000x |

---

## Per-Scenario Details
### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| PatterniaPipe | 1.774 | +32.09% | - | 0.46 |
| IfElse | 1.898 | +41.37% | +7.02% | 3.70 |
| Switch | 1.343 | fastest | -24.29% | 0.69 |
| StdVisit | 2.015 | +50.08% | +13.62% | 0.51 |

### LiteralMatch

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| PatterniaPipe | 1.800 | +99.74% | - | 0.10 |
| IfElse | 0.908 | +0.75% | -49.56% | 0.25 |
| Switch | 0.901 | fastest | -49.94% | 1.45 |

### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| PatterniaPipe | 1.405 | fastest | - | 3.48 |
| Switch | 1.436 | +2.25% | +2.25% | 1.47 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| PatterniaPipe | 1.671 | +12.02% | - | 1.03 |
| IfElse | 1.888 | +26.59% | +13.01% | 0.65 |
| Switch | 1.723 | +15.49% | +3.10% | 0.22 |
| StdVisit | 1.492 | fastest | -10.73% | 0.28 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| PatterniaPipe | 1.069 | fastest | - | 0.72 |
| SwitchIndex | 1.086 | +1.60% | +1.60% | 0.73 |
| StdVisit | 1.071 | +0.14% | +0.14% | 0.81 |
| Sequential | 1.233 | +15.31% | +15.31% | 0.26 |

