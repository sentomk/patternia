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

**Patternia** addresses the limitations discussed above by providing a structured and declarative way to express control flow directly in terms of data shape. Instead of organizing logic around condition checks and casts, Patternia allows developers to describe *what kind of data is expected* in each branch and to introduce bindings only when a match succeeds. This makes each branch self-contained and aligns control flow with the semantic structure of the data.

Patternia is designed as a library-level abstraction that integrates naturally with existing C++ code. It does not require changes to data definitions, does not impose a closed-world assumption, and does not rely on runtime reflection. By explicitly separating matching, binding, and guards, Patternia avoids hidden side effects while remaining flexible enough to model both Rust-like pattern matching semantics and C++-specific use cases.

```cpp
// Patternia-style control flow (conceptual)
match(value)
  .when(Point { x, y } if x > 0 >> handle(x, y))
  .when(Line  { start, end }   >> handle(start, end))
  .otherwise(fallback);
```


## Quick Start


## API Reference


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
