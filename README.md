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


## *Learn Patternia*

### Getting Started

- **[Quick Start](https://patternia.tech/guide/getting-started/)**  
  Install Patternia and write your first pattern match case.


### Tutorials

#### Getting Into Pattern Matching

- **[From Control Flow to Pattern Matching](https://patternia.tech/tutorials/from-control-flow/)**  
  Refactoring `if` / `switch` into declarative pattern-based logic.

#### For Readers Familiar with Pattern Matching


- **[Pattern Matching in Other Languages](https://patternia.tech/tutorials/other-languages)**  
  How Patternia relates to pattern matching systems in other languages.

#### Core Techniques

- **[Custom Predicate Guards](https://patternia.tech/tutorials/predicate-guards)**  
  Defining and using custom guards in `when(...)`.

#### Worked Examples

- **[Policy Constraint Matching](https://patternia.tech/tutorials/policy-constraint)**  
  Expressing rule-based access policies as declarative constraints.

- **[Geometric Constraint Matching](https://patternia.tech/tutorials/sphere-constraint)**  
  Expressing `x² + y² + z² < 1` as a declarative pattern.


## *What Is Pattern Matching*  

**Pattern matching** is a control-flow mechanism that selects execution paths based on the structural form, construction, or type of values. By combining discrimination, decomposition, and structured binding into a single construct, pattern matching allows programs to reason about data shapes directly, rather than through ad-hoc conditional logic. Compared to traditional if-else chains or switch statements, pattern matching offers a more declarative and data-oriented way to express branching logic.

**Further reading:**

* [Haskell Pattern Matching](https://www.haskell.org/tutorial/patterns.html)
* [Rust Pattern Matching](https://doc.rust-lang.org/book/ch19-00-patterns.html)
* [Scala Match Expressions](https://docs.scala-lang.org/tour/pattern-matching.html)
* [Python Structural Pattern Matching (PEP 634)](https://peps.python.org/pep-0634/)


## *Control Flow in C++*

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

## *What Patternia Solves*

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


## *Quick Start*

Patternia is a **header-only** library that brings expressive pattern matching to modern C++. 

**Key Features:**

* **Zero-overhead** abstraction with compile-time optimization
* **Expression-oriented** matching with value-returning capabilities
* **Structural pattern matching** for complex data shapes
* **Composable guard system** for conditional logic
* **Type-safe** bindings and explicit data flow

**Quick Example:**

```cpp
#include <ptn/patternia.hpp>

int classify(int x) {
  return ptn::match(x)
    .when(ptn::lit(0) >> 0)
    .when(ptn::lit(1) >> 1)
    .otherwise(-1);
}
```

For complete installation instructions, comprehensive examples, and in-depth tutorials, visit the **[Getting Started Guide](https://sentomk.github.io/patternia/guide/getting-started/)**.


## *API Reference*

### Quick Navigation

> Jump directly to a specific part of the Patternia API.

---

### I. Match DSL Core Framework

* [`match(subject)`](https://sentomk.github.io/patternia/api/#matchsubject)
* [`match(subject, case_tuple(...))`](https://sentomk.github.io/patternia/api/#matchsubject-case_tuple)
* [`.when(pattern >> handler)`](https://sentomk.github.io/patternia/api/#whenpattern-handler)
* [`.otherwise(...)`](https://sentomk.github.io/patternia/api/#otherwise)
* [`.end()`](https://sentomk.github.io/patternia/api/#end )
* [Fallback Mechanisms Comparison](https://sentomk.github.io/patternia/api/#comparison-summary)

---

### II. Pattern Primitives

* [`lit(value)`](https://sentomk.github.io/patternia/api/#litvalue)
* [`__` (Wildcard)](https://sentomk.github.io/patternia/api/#__-wildcard)
* [`bind()`](https://sentomk.github.io/patternia/api/#binding-the-entire-subject)
* [`bind(subpattern)`](https://sentomk.github.io/patternia/api/#conditional-binding-with-bindsubpattern)
* [Binding Semantics & Order](https://sentomk.github.io/patternia/api/#binding-semantics)

---

### III. Guard System

* [`[]` Guard Attachment](https://sentomk.github.io/patternia/api/#guard-attachment)
* [`_` Placeholder (Single-value Guards)](https://sentomk.github.io/patternia/api/#single-value-guards-_-and-rng)
* [`rng(...)` Range Guards](https://sentomk.github.io/patternia/api/#range-helper-rnglo-hi-mode)
* [`arg<N>` (Multi-value Guards)](https://sentomk.github.io/patternia/api/#multi-value-guards-argn-expressions)
* [Custom Predicate Guards (Lambda)](https://sentomk.github.io/patternia/api/#custom-predicates-recommended-for-domain-logic)

---

### IV. Structural Matching

* [`has<&T::member...>`](https://sentomk.github.io/patternia/api/#hastmember)
* [Structural Constraints](https://sentomk.github.io/patternia/api/#core-characteristics)
* [Structural Binding with `bind()`](https://sentomk.github.io/patternia/api/#structural-binding-with-has)
* [Partial Structural Matching](https://sentomk.github.io/patternia/api/#partial-structural-matching)
* [Design Rationale & Guarantees](https://sentomk.github.io/patternia/api/#design-rationale)

---

### V. Namespace Structure

* [Namespace Layout](https://sentomk.github.io/patternia/api/#v-namespace-structure)
* [Primary Namespace Aliases](https://sentomk.github.io/patternia/api/#primary-namespace-aliases)

---

> **Note**
> This API reference documents Patternia’s *language constructs*, not just function signatures.
> Each section explains the semantic role and design intent of the API, in addition to usage examples.


## *Contributing*

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

---

<div align="center">
  <sub>Built with ❤️ for modern C++ development</sub>
</div>
