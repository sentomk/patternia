# Patternia

Patternia is a modern C++ pattern-matching DSL designed to make dispatch logic
clear, expressive, and type-safe — without sacrificing performance or compilation efficiency.

It provides a structured way to express matching rules through composable patterns,
including value patterns, type patterns, and extensible DSL operators.

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

Patternia evaluates patterns in order and returns the result associated with the
first successful match.

---

## Why Patternia

* Header-only, zero dependencies
* Declarative matching style
* Strongly typed DSL
* Composable value, type, and predicate patterns
* Clean chaining model: `match().when().otherwise()`
* Ready for integration into modern C++ projects

---

## Core Concepts

Patternia organizes matching logic into a simple layered model:

* **Value Patterns**
  Match literals, ranges, and custom predicates.

* **Type Patterns**
  Match exact types, type lists, or template families.

* **Variant Patterns** *(in development)*
  Structured matching for `std::variant` and user-defined sum types.

* **DSL Operators**
  `>>` (pattern → handler), `&&`, `||`, `!` for pattern composition.

---

## Get Started

To begin using Patternia, see:

* [Installation](guide/installation.md)
* [Getting Started](guide/getting-started.md)
* [Concepts](guide/concepts.md)

---

## Explore the Pattern System

* [Pattern System Architecture](patterns/foreword.md)
* [Value Patterns](patterns/value.md)
* [Type Patterns](patterns/type.md)

---

## API Reference

* [match()](api/match.md)
* [value patterns](api/value.md)
* [type patterns](api/type.md)
* [variant](api/variant.md)
* [DSL Operators](api/dsl.md)

---

## Project

* [Roadmap](design/roadmap.md)
* [Releases](changelog/releases.md)
* [GitHub Repository](https://github.com/sentomk/patternia)

---

Patternia aims to make logical structure explicit —
so your C++ code becomes easier to read, reason about, and extend.
  