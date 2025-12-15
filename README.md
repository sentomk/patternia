<div align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" 
           srcset="https://wordpress-1316673449.cos.ap-beijing.myqcloud.com/img/banner-dark.svg">
    <source media="(prefers-color-scheme: light)" 
            srcset="https://wordpress-1316673449.cos.ap-beijing.myqcloud.com/img/banner-dark.svg">
    <img alt="Patternia logo"
         width="300"
         style="max-width: 90%; height: auto; margin-top: 10px; transform: translateX(-2px);">
  </picture>
</div>

<br>

<div align="center">
  
[![C++17+](https://img.shields.io/badge/C%2B%2B-17%2B-blue.svg?style=flat-square&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Build](https://github.com/SentoMK/patternia/actions/workflows/ci.yml/badge.svg?branch=main&style=flat-square)](https://github.com/SentoMK/patternia/actions)
[![License](https://img.shields.io/github/license/SentoMK/patternia?style=flat-square)](LICENSE)
[![Version](https://img.shields.io/github/v/release/SentoMK/patternia?style=flat-square&color=orange)](https://github.com/SentoMK/patternia/releases)
[![Docs](https://img.shields.io/badge/docs-online-blue?style=flat-square)](https://sentomk.github.io/patternia)

</div>

<br>

**A header-only, zero-overhead pattern matching library for modern C++ that transforms branching logic into expressive, maintainable code.**


## What Is Pattern Matching  

**Pattern matching** is a control-flow mechanism that selects execution paths based on the structural form, construction, or type of values. By combining discrimination, decomposition, and structured binding into a single construct, pattern matching allows programs to reason about data shapes directly, rather than through ad-hoc conditional logic. Compared to traditional if-else chains or switch statements, pattern matching offers a more declarative and data-oriented way to express branching logic.

**Further reading:**

* [Haskell Pattern Matching](https://www.haskell.org/tutorial/patterns.html)
* [Rust Pattern Matching](https://doc.rust-lang.org/book/ch19-00-patterns.html)
* [Scala Match Expressions](https://docs.scala-lang.org/tour/pattern-matching.html)
* [Python Structural Pattern Matching (PEP 634)](https://peps.python.org/pep-0634/)


## Control Flow in C++

In modern C++, control flow over heterogeneous or structured data is typically expressed using a combination of `if`/`else`, `switch`, type checks, and manual data access. While these mechanisms are flexible and expressive, they tend to scale poorly as data structures become more complex or evolve over time. Type discrimination, structural access, and business logic are often interleaved, making the resulting control flow harder to read, maintain, and extend.

```cpp
// Conventional control flow (conceptual)
if (value is TypeA) {
    auto& a = as<TypeA>(value);
    if (a.field > threshold) {
        ...
    }
} else if (value is TypeB) {
    auto& b = as<TypeB>(value);
    ...
} else {
    ...
}
```

In this style, control flow is organized around **conditions and checks**, rather than around the structure of the data itself. The logic for discriminating types, accessing fields, and introducing local variables is scattered across branches, often leading to duplicated checks and implicit assumptions about data invariants.

With pattern matching, control flow can instead be organized around **data shapes**. Each branch explicitly states the structure it expects and introduces bindings only when the match succeeds. This aligns control flow more closely with the semantics of the data and makes each branch self-contained.

```cpp
// Control flow with pattern matching (conceptual)
match value {
    when PatternA(x) if x > threshold => {
        ...
    }
    when PatternB(y) => {
        ...
    }
    otherwise => {
        ...
    }
}
```

By unifying discrimination, decomposition, and binding, pattern matching allows control flow to be expressed declaratively and locally. This approach reduces boilerplate, minimizes accidental complexity, and provides a clearer foundation for reasoning about completeness and correctness as data structures evolve.

## What Patternia Solves

**Patternia** is designed to address several long-standing pain points in C++ control flow and data-oriented logic—especially in codebases that operate on evolving data structures, heterogeneous types, or complex branching rules.

### 1. Scattered Control Logic over Structured Data

In idiomatic C++, logic that depends on the *shape* or *internal structure* of data is typically expressed through a mix of:

* type checks (`std::variant`, `dynamic_cast`, tag fields),
* manual field access,
* nested conditionals,
* and ad-hoc invariants enforced implicitly by control flow.

As a result, **structural assumptions about data are rarely explicit**, and reasoning about correctness often requires reading across multiple branches.

Patternia allows control flow to be written *in terms of structure*:

```cpp
match(p)
  .when(bind(has<&Point::x, &Point::y>()) >> [](int x, int y) {
    // explicitly operates on {x, y}
  })
  .otherwise(...);
```

Each branch clearly states *what shape of data it expects* and *what it binds*, making invariants explicit and local.

---

### 2. Interleaving Decomposition and Conditional Logic

In conventional C++, decomposition (extracting fields) and conditional checks are usually interleaved:

```cpp
if (p.x + p.y == 0 && p.x > 0) {
  ...
}
```

As conditions grow more complex—especially when they involve **relationships between multiple values**—this style becomes increasingly opaque.

Patternia separates these concerns:

* **Patterns** describe *how data is decomposed*.
* **Guards** describe *constraints over the bound values*.

```cpp
match(p)
  .when(
    bind(has<&Point::x, &Point::y>())[arg<0> + arg<1> == 0] >>
    [](int x, int y) { ... }
  );
```

This separation improves readability and enables richer forms of reasoning over multi-value relationships.

---

### 3. Lack of Expressive, Composable Guards

Traditional control flow relies on raw boolean expressions, which:

* do not compose well,
* cannot be reused independently of control flow,
* and provide no structural context.

Patternia introduces **first-class guard expressions** that are:

* composable (`&&`, `||`),
* expressive (relational, arithmetic, predicate-based),
* and structurally aware (single-value and multi-value guards).

```cpp
bind()[_ > 0 && _ < 10]
bind(has<&A::x, &A::y>())[arg<0> * arg<1> > 100]
```

Guards become part of the pattern language rather than incidental conditions.

---

### 4. Unifying Value-Based and Structural Branching

C++ provides multiple mechanisms for branching:

* `switch` for integral values,
* `if constexpr` for compile-time decisions,
* `std::visit` for variants,
* and manual destructuring for aggregates.

These mechanisms are **orthogonal and non-uniform**, often forcing developers to mix paradigms within the same function.

Patternia offers a single abstraction that can express:

* literal matching,
* relational matching,
* structural decomposition,
* guarded constraints,
* and fallback behavior

within one coherent, expression-oriented model.

---

### 5. Expression-Oriented Matching with Zero Overhead

Patternia treats pattern matching as an **expression**, not just a control-flow statement:

```cpp
auto result = match(n)
  .when(lit(0) >> 0)
  .when(lit(1) >> 1)
  .otherwise([] { return compute(); });
```

At the same time, it adheres strictly to C++’s zero-overhead principle:

* no runtime type erasure,
* no virtual dispatch,
* no heap allocation,
* no hidden control flow.

All matching logic is resolved through templates and inlined calls, allowing the compiler to optimize aggressively.

---

### 6. Making Data Shape a First-Class Concept

Ultimately, Patternia is not about replacing `if` or `switch`.
It is about elevating **data shape**—structure, relationships, and constraints—to a first-class concept in C++ control flow.

This makes Patternia particularly suitable for:

* state machines,
* protocol handling,
* geometric and numeric logic,
* AST processing,
* rule-based systems,
* and any domain where *what the data looks like* matters as much as *what its value is*.


## Quick Start

### 1. Installation

Patternia is a **header-only** library. No compilation or binary linking is required.

#### Using CMake FetchContent (Recommended)

Patternia can be integrated directly via CMake `FetchContent`. This is the recommended approach during active development.

```cmake
include(FetchContent)

FetchContent_Declare(
  patternia
  GIT_REPOSITORY https://github.com/SentoMK/patternia.git
  GIT_TAG        main
)

FetchContent_MakeAvailable(patternia)
```

Then, include Patternia headers in your code:

```cpp
#include <ptn/patternia.hpp>
```

**Notes:**

* `GIT_TAG main` tracks the latest development version
* Patternia is header-only — no targets need to be linked
* Requires **C++17 or later**



#### By vcpkg

vcpkg support is **coming soon**.


#### Other Installation Methods

For additional installation options and detailed setup instructions (including manual integration and future package manager support), see:

👉 [Patternia Installation](https://sentomk.github.io/patternia/guide/installation/)


### 2. Matching Values

At its simplest, Patternia replaces `if` / `switch` with a declarative, ordered match expression.

```cpp
#include <iostream>
#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int x = 2;

  match(x)
    .when(lit(1) >> [] { std::cout << "one\n"; })
    .when(lit(2) >> [] { std::cout << "two\n"; })
    .otherwise([] { std::cout << "other\n"; });
}
```

**Key points:**

* `match(subject)` starts a matching expression
* `lit(v)` matches a literal value using `==`
* Cases are tested **top-to-bottom (first-match semantics)**
* `otherwise` acts as a fallback


### 3. Expression-Oriented Matching

Patternia supports **value-returning matches**, similar to Rust or Scala.

```cpp
int classify(int x) {
  return match(x)
    .when(lit(0) >> 0)
    .when(lit(1) >> 1)
    .otherwise(-1);
}
```

A match expression either:

* returns a value (`otherwise(value_or_lambda)`), or
* performs side effects (`.end()`)


### 4. Binding Values

To access the matched value inside a handler, use `bind()`.

```cpp
match(x)
  .when(bind() >> [](int v) {
    std::cout << "value = " << v << "\n";
  })
  .otherwise([] {});
```

`bind()` introduces bindings explicitly—nothing is bound implicitly.

This makes data flow **visible and predictable**, especially in complex matches.


### 5. Guards (Conditional Matching)

Guards allow you to attach constraints to patterns.

```cpp
using namespace ptn;

match(x)
  .when(bind()[_ > 0 && _ < 10] >> [](int v) {
    std::cout << "in range: " << v << "\n";
  })
  .otherwise([] {
    std::cout << "out of range\n";
  });
```

* `_` is a placeholder for the bound value
* Guard expressions are **composable** and **side-effect free**
* Guards run only after the pattern itself matches


### 6. Structural Matching

Patternia can match **structure**, not just values.

```cpp
struct Point {
  int x;
  int y;
};

Point p{3, -3};

match(p)
  .when(
    bind(has<&Point::x, &Point::y>())[arg<0> + arg<1> == 0] >>
    [](int x, int y) {
      std::cout << "on diagonal\n";
    }
  )
  .otherwise([] {});
```

Here:

* `has<&Point::x, &Point::y>` describes the expected **shape**
* `bind(...)` extracts values explicitly
* Guards can express **relationships between multiple bindings**


### 7. Wildcard Matching

Use `__` to match anything without binding.

```cpp
match(x)
  .when(lit(0) >> [] { std::cout << "zero\n"; })
  .when(__ >> [] { std::cout << "anything else\n"; });
```


### 8. When to Use Patternia

Patternia is particularly effective when:

* branching depends on **data shape**, not just values
* conditions involve **relationships between multiple fields**
* `if` / `switch` logic becomes deeply nested
* you want expression-oriented control flow

For a deeper dive, see the **API Reference** and **Design Guide**.


## API Reference

### Quick Navigation

> Jump directly to a specific part of the Patternia API.

---

### I. Match DSL Core Framework

* [`match(subject)`]()
* [`.when(pattern >> handler)`]()
* [`.otherwise(...)`]()
* [`.end()`]()
* [Expression vs Statement Matching]()

---

### II. Pattern Primitives

* [`lit(value)`]()
* [`lit_ci(value)`]()
* [`__` (Wildcard)]()
* [`bind()`]()
* [`bind(subpattern)`]()
* [Binding Semantics & Order]()

---

### III. Guard System

* [`[]` Guard Attachment]()
* [`_` Placeholder (Single-value Guards)]()
* [`rng(...)` Range Guards]()
* [`arg<N>` (Multi-value Guards)]()
* [Custom Predicate Guards (Lambda)]()
* [Guard Evaluation Semantics]()

---

### IV. Structural Matching

* [`has<&T::member...>`]()
* [Structural Constraints]()
* [Structural Binding with `bind()`]()
* [Partial Structural Matching]()
* [Design Rationale & Guarantees]()

---

### V. Namespace Structure

* [Namespace Layout]()
* [Primary Namespace Aliases]()
* [User-facing API Surface]()

---

> **Note**
> This API reference documents Patternia’s *language constructs*, not just function signatures.
> Each section explains the semantic role and design intent of the API, in addition to usage examples.


## Contributing

Contributions are welcome. Whether it is **bug reports**, **feature proposals**, or **pull requests**, your help is appreciated.

### Before Submitting Changes

1. **Discuss First**
   Open an Issue or a Discussion when proposing new features or behavior changes.
2. **Code Style**
   Follow modern C++ idioms and ensure:

   * C++17+ compatible
   * No RTTI or virtual dispatch introduced
   * Zero-overhead principles preserved
3. **Tests Required**
   Add or update unit tests under `tests/` for any logic changes.
4. **Comment Style**
   Follow **[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html#Comment_Style)** for documentation comments.
5. **Commit Style**
   Use conventional commit messages:

   ```
   feat: add case-insensitive string matching
   fix: correct match_result type inference
   refactor: reorganize dsl ops
   ```

### Development Setup

```bash
git clone https://github.com/SentoMK/patternia.git
cd patternia
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Run tests:

```bash
ctest --test-dir build
```

---

<div align="center">
  <sub>Built with ❤️ for modern C++ development</sub>
</div>
