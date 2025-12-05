# Patternia Documentation

**A modern C++ pattern matching DSL**

Patternia is a modern C++ pattern-matching DSL designed to make dispatch logic clear, expressive, and type-safe — without sacrificing performance or compilation efficiency.

---

## Navigation

### Quick Start
- [Installation](guide/installation.md) - How to integrate Patternia into your project
- [Getting Started](guide/getting-started.md) - Complete examples from basics to advanced

### Pattern System
- [Pattern System Overview](patterns/foreword.md) - Understand Patternia's design philosophy
- [Value Patterns](patterns/value.md) - Value matching with literals, ranges, predicates
- [Type Patterns](patterns/type.md) - Type matching with type checking, type lists

### API Reference
- [API Documentation](api/api.md) - Complete API reference for all functions and patterns

### Project Information
- [Roadmap](design/roadmap.md) - Future development directions and plans
- [Releases](changelog/releases.md) - Version update history

---

## Core Features

- **Header-only**: Zero dependencies, plug-and-play
- **Declarative Syntax**: `match().when().otherwise()` chaining
- **Type Safety**: Compile-time guarantees, zero runtime overhead
- **Rich Patterns**: Value patterns, type patterns, predicate patterns
- **DSL Operators**: `>>`, `&&`, `||`, `!` for composition

---

## Quick Example

```cpp
#include <ptn/patternia.hpp>
using namespace ptn;

int x = 42;

auto result =
    match(x)
      .when(lit(0) >> "zero")
      .when(lit(42) >> "answer")
      .when(type::is<int> >> "int value")
      .otherwise("other");
```

Patternia evaluates patterns in order and returns the result associated with the first successful match.

---

## Where to Start

If you're **new to Patternia**, we recommend reading in this order:

1. [Installation](guide/installation.md) - Integrate into your project
2. [Getting Started](guide/getting-started.md) - Learn basic usage
3. [Value Patterns](patterns/value.md) or [Type Patterns](patterns/type.md) - Deep dive based on your needs

If you're looking for **specific information**:

- Need a function reference? → [API Documentation](api/api.md)
- Want to understand design philosophy? → [Pattern System Overview](patterns/foreword.md)
- Check version updates? → [Releases](changelog/releases.md)

---

## System Requirements

- **C++ Standard**: C++17 or later
- **Compilers**: GCC ≥11, Clang ≥12, MSVC ≥2019
- **Platforms**: Linux, Windows, macOS

---

<div align="center">

<b>Make logical structure explicit, make C++ code more elegant</b>

</div>
