# Patternia Benchmark Report

- Source: `bench_results/ptn_bench.json`
- Scenarios: `4`
- Patternia fastest: `2/4`
- Average Patternia gap vs fastest: `+2.76%`
- Largest Patternia gap: `PacketMixed` `+10.69%` vs `Switch`

## Patternia Focus

| Scenario | Patternia impl | Rank | Patternia mean (ns) | Fastest | Fastest mean (ns) | Gap vs fastest | Patternia CV % | Status |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| PacketMixed | PatterniaPipe | 2/2 | 1.622 | Switch | 1.466 | +10.69% | 0.23 | watch |
| VariantMixed | PatterniaPipe | 2/2 | 2.000 | StdVisit | 1.993 | +0.36% | 0.86 | close |
| CommandParser | PatterniaPipe | 1/2 | 1.775 | PatterniaPipe | 1.775 | +0.00% | 0.59 | fastest |
| ProtocolRouter | PatterniaPipe | 1/2 | 1.628 | PatterniaPipe | 1.628 | +0.00% | 0.48 | fastest |

---

## Per-Scenario Details
### PacketMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| Switch | 1.466 | fastest | -9.66% | 1.57 |
| **PatterniaPipe** | 1.622 | +10.69% | - | 0.23 |

### VariantMixed

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| StdVisit | 1.993 | fastest | -0.36% | 0.45 |
| **PatterniaPipe** | 2.000 | +0.36% | - | 0.86 |

### CommandParser

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.775 | fastest | - | 0.59 |
| Switch | 2.252 | +26.87% | +26.87% | 0.16 |

### ProtocolRouter

| Impl | Mean (ns) | vs fastest | vs Patternia | CV % |
|---|---:|---:|---:|---:|
| **PatterniaPipe** | 1.628 | fastest | - | 0.48 |
| IfElse | 1.915 | +17.64% | +17.64% | 4.63 |

