# Single Benchmark Report

- Source: `/home/runner/work/patternia/patternia/bench_results/ptn_bench.json`

## CommandParser

| Impl | Mean (ns) | Delta vs fastest | CV % |
|---|---:|---:|---:|
| IfElse | 1.818 | +34.43% | 1.67 |
| Switch | 1.352 | +0.00% | 0.57 |
| StdVisit | 1.629 | +20.45% | 0.70 |
| PatterniaPipe | 1.772 | +31.07% | 0.13 |

## LiteralMatch

| Impl | Mean (ns) | Delta vs fastest | CV % |
|---|---:|---:|---:|
| IfElse | 0.827 | +0.93% | 1.53 |
| Switch | 0.819 | +0.00% | 0.45 |
| PatterniaPipe | 1.390 | +69.72% | 0.28 |

## PacketMixed

| Impl | Mean (ns) | Delta vs fastest | CV % |
|---|---:|---:|---:|
| Switch | 1.643 | +13.03% | 1.63 |
| PatterniaPipe | 1.453 | +0.00% | 5.51 |

## ProtocolRouter

| Impl | Mean (ns) | Delta vs fastest | CV % |
|---|---:|---:|---:|
| IfElse | 1.888 | +26.94% | 0.28 |
| Switch | 1.723 | +15.83% | 0.38 |
| StdVisit | 1.488 | +0.00% | 0.53 |
| PatterniaPipe | 1.650 | +10.91% | 0.30 |

## VariantMixed

| Impl | Mean (ns) | Delta vs fastest | CV % |
|---|---:|---:|---:|
| SwitchIndex | 0.997 | +7.31% | 1.59 |
| StdVisit | 1.107 | +19.05% | 0.20 |
| PatterniaPipe | 0.930 | +0.00% | 0.60 |
| Sequential | 1.145 | +23.19% | 0.28 |

