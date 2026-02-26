# Patternia Documentation

**A modern C++ pattern matching DSL**

Patternia is a modern C++ pattern-matching DSL designed to make dispatch logic clear, expressive, and type-safe — without sacrificing performance or compilation efficiency.

---

## Navigation

### Start Here
- [Getting Started](guide/getting-started.md) - Minimal examples and core syntax
- [Installation](guide/installation.md) - How to integrate Patternia into your project

### Concepts and Tutorials
- [From Control Flow to Pattern Matching](tutorials/from-control-flow.md) - Onramp for new users
- [Pattern Matching in Other Languages](tutorials/other-languages.md) - Mapping familiar ideas to Patternia
- [Custom Predicate Guards](tutorials/predicate-guards.md) - Guard design and composition

### Worked Examples
- [Policy Constraint Matching](tutorials/policy-constraint.md) - Declarative rules and access control
- [Geometric Constraint Matching](tutorials/sphere-constraint.md) - Numeric constraints as patterns

### Reference
- [API Documentation](api.md) - Complete API reference for all patterns and functions
- [Performance Notes](performance/index.md) - Algorithm evolution and dispatch strategy by version
- [Design Overview](design-overview.md) - Semantics and matching model

### Project Information
- [Releases](changelog/releases.md) - Version update history
- [Benchmark Scripts](https://github.com/SentoMK/patternia/blob/main/scripts/README.md) - JSON compare and visualization tooling

---

## System Requirements

- **C++ Standard**: C++17 or later
- **Compilers**: GCC ≥11, Clang ≥12, MSVC ≥2019
- **Platforms**: Linux, Windows, macOS

---

## Benchmarking

- Run benchmark suite from project root:

```powershell
.\build\bench\ptn_bench.exe --benchmark_filter="Variant" --benchmark_out=build/variant_all.json --benchmark_out_format=json
```

- Visualize one JSON across implementations:

```powershell
py -3 scripts/bench_single_report.py --input build/variant_all.json --include "Variant" --outdir build/bench/single --prefix variant_single
```

---

<div align="center">

<b>Make logical structure explicit, make C++ code more elegant</b>

</div>
