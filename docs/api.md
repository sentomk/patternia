# Patternia API Reference

## Introduction

Patternia provides a comprehensive pattern matching system for C++ with a focus on type safety, performance, and expressiveness. This document outlines the core APIs that constitute Patternia's pattern matching language.

---

## I. Match DSL Core Framework

The Match DSL forms the foundation of Patternia.  
It defines how values are inspected, how cases are registered, and how matching is ultimately evaluated.

---

### `match(subject)`

**Role**: DSL entry point and evaluation context for the entire system.

**Syntax**:
```cpp
match(subject)
```

**Core Characteristics**:

* `subject` is the value being matched, not an implicitly captured variable
* `match()` returns a builder object; no matching is performed immediately
* Cases are evaluated sequentially using **first-match semantics**

**Basic Usage**:

```cpp
match(x) | on(
  lit(1) >> [] { return "one"; },
  __ >> "default"
);
```

**Type Control**:

Patternia currently matches the subject as its actual type; there is no
`match<AsType>(subject)` override.

---

### `match(subject) | on(...)` {#matchsubject-on}

**Role**: Primary API for Patternia v0.8.x.

**Syntax**:
```cpp
match(subject) | on(case1, case2, ...)
```

**Key Characteristics**:

* Pipeline-style terminal form, evaluated immediately
* Cases are evaluated sequentially using **first-match semantics**
* Requires a pattern fallback `__` in the `on(...)` case list
* Supports full pattern capabilities including guards and bindings

**Basic Usage**:

```cpp
auto r = match(x) | on(
  lit(1) >> [] { return "one"; },
  lit(2) >> [] { return "two"; },
  __     >> [] { return "other"; }
);
```

**Compatibility Note**:

- `match(subject, cases(...)).end()` was removed in v0.8.0.
- Migrate legacy compact forms to `match(subject) | on(...)`.

---

### Case Expression: `pattern >> handler`

**Role**: Core case-definition primitive used within `on(...)`.

**Syntax**:

```cpp
pattern >> handler
```

**Key Characteristics**:

* `pattern >> handler` forms a *case expression*
* The handler's parameter list is determined entirely by the pattern's binding behavior
* Multiple cases are separated by commas within `on(...)`

**Usage Examples**:

```cpp
// Handler without bindings
lit(1) >> [] { return "one"; }   // No parameters
lit(2) >> 42                     // Constant value handler

// Handler with bindings
$(is<int>()) >> [](int v) { return v * 2; }

// Structural bindings
$(has<&Point::x, &Point::y>()) >> [](int x, int y) { return x + y; }
```

**Handler Forms**:

* **Value Handler** - `pattern >> value`
  Returns a fixed value. Semantically equivalent to a zero-argument function returning a constant.

* **Function Handler** - `pattern >> callable`
  Receives the values produced by the pattern's bindings.

---

## II. Pattern Primitives

Pattern primitives constitute the core vocabulary of Patternia’s pattern language.  
They describe *what* is matched, *what* is ignored, and *what* is explicitly bound.

---

### `lit(value)` and `lit<value>()`

**Role**: Fundamental value patterns for exact matching.

**Syntax**:
```cpp
template <typename V>
constexpr auto lit(V &&v);      // Runtime value pattern

template <auto V>
constexpr auto lit();           // Compile-time value pattern

template <typename V>
constexpr auto lit_ci(V &&v);   // Runtime ASCII case-insensitive match
```

**Semantics**:

* `lit(value)` stores a runtime value and matches with `operator==`
* `lit<value>()` encodes the value in the type and is the entry point for
  static literal lowering
* `lit_ci(value)` performs ASCII case-insensitive matching for string-like
  values
* `lit_ci()` intentionally remains a runtime factory; there is no
  `lit_ci<value>()` static counterpart
* All three produce no bindings

**When To Use Which**:

* Use `lit(value)` for general matching, dynamic values, and string literals
* Use `lit<value>()` when the literal is known at compile time and you want the
  lowering engine to consider switch-like optimization paths
* Use `lit_ci(value)` for runtime ASCII case-insensitive string matching

**Examples**:

```cpp
.when(lit(Status::Running) >> ...)   // General enum/value matching
.when(lit<42>() >> ...)              // Compile-time integer literal
.when(lit("hello") >> ...)          // Runtime string literal
.when(lit_ci("hello") >> ...)       // Case-insensitive string match
```

**Supported Types**:

* Arithmetic types (`int`, `double`, `float`)
* Enumeration types
* String types (`std::string`, `std::string_view`, `const char*`)
* User-defined types providing `operator==`

---

### `__` (Wildcard)

**Role**: Pattern-level wildcard that matches any value without binding.

**Syntax**:

```cpp
inline constexpr detail::wildcard_t __;
```

**Key Properties**:

* Always matches
* Introduces no bindings
* Participates in pattern ordering and exhaustiveness

```cpp
.when(__ >> [] {
  // No access to the matched value
})
```

**Design Intent**:

```cpp
match(value)
  .when(lit("success") >> "ok")
  .when(lit("error")   >> "fail")
  .when(__             >> "unknown")  // Pattern-level fallback
  .end();
```

**Key Distinction**:

* `__` - a pattern that participates in matching and ordering
* `.otherwise()` - a match-level fallback executed only if no pattern matches
* `__` and `.otherwise()` cannot be combined in the same match

---

### `is<T>()` (Variant Type Matching)

**Role**: Match `std::variant` alternatives by type, with optional subpattern.

**Syntax**:
```cpp
is<T>()                 // type-only match
is<T>(subpattern)       // apply subpattern to the alternative
```

**Key Properties**:

* Works on `std::variant` subjects only
* `is<T>()` does not bind values by itself
* Use `$(is<T>())` for explicit binding of the extracted alternative
* When a type pattern binds (e.g. `$(is<T>())`, or `is<T>(bind(...))`), it is a **binding pattern** and can be guarded with `[]`
* Alternative type `T` must appear exactly once in the variant

**Examples**:

```cpp
match(v) | on(
  is<int>() >> [] { /* type-only */ },
  $(is<std::string>()) >> [](const std::string &s) { /* bound */ },
  $(is<std::string>())[_0 != ""] >> [](const std::string &s) { /* guarded */ },
  is<Point>($(has<&Point::x, &Point::y>())) >> [](int x, int y) { /* structural bind */ },
  __ >> [] {}
);
```

**Design Note**:
`$(is<T>())` preserves the "explicit bind" rule by using the `$()` callable syntax,
making binding intent clear and consistent with other binding patterns.


---

### `$()` and `bind()` (Binding Patterns)

**Role**: Explicit value binding primitives.

Patternia provides two ways to introduce bindings:
- `$()` - Convenient callable syntax (recommended)
- `bind()` - Underlying primitive

**Syntax**:

```cpp
// $ callable forms
$()                    // Binds the entire subject
$(subpattern)          // Wraps subpattern with binding

// bind() forms
bind()                 // Binds the entire subject
bind(subpattern)       // Binds subject conditionally under a subpattern constraint
```

**Equivalences**:

```cpp
$()              ≡ bind()
$(has<&T::x>())  ≡ bind(has<&T::x>())
$(is<T>())       ≡ bind(is<T>())
```

**Core Principles**:

1. All bindings are explicit
2. No pattern introduces bindings implicitly
3. Binding behavior fully determines handler parameter lists
4. `$()` is the recommended syntax for clarity

---

#### Binding the Entire Subject

```cpp
match(x) | on(
  $() >> [](auto v) { return "captured: " + std::to_string(v); },
  __ >> "default"
);
```

* The entire subject is bound as a single value
* The handler receives exactly one parameter

---

#### Conditional Binding with `$(subpattern)`

```cpp
match(status) | on(
  $(lit(Status::Running)) >> [](Status s) {
    return fmt("status = {}", static_cast<int>(s));
  },
  __ >> "unknown"
);
```

* `$(subpattern)` introduces a binding **only if the subpattern matches**
* The bound value is always the **entire subject**
* The subpattern itself does **not** introduce bindings unless explicitly designed to do so

This form is useful when:

* a value must be both **filtered** and **captured**
* the captured value needs to be reused by the handler
* the constraint is semantic rather than structural


#### Structural Binding with `$(has<>())`

```cpp
struct Point { int x, int y; };

match(p) | on(
  $(has<&Point::x, &Point::y>()) >> [](int x, int y) {
    return fmt("({}, {})", x, y);
  },
  __ >> "invalid"
);
```

* `has<>` describes the required structure
* `$()` wraps it to enable binding
* Only the listed members are bound
* No access to unlisted members is provided

Partial binding is expressed by listing only the desired members:

```cpp
match(p) | on(
  $(has<&Point::x>()) >> [](int x) { return fmt("x = {}", x); },
  __ >> "invalid"
);
```

#### Binding Semantics

* `$()` and `bind()` always bind the *entire subject*
* `$(subpattern)` and `bind(subpattern)` bind the subject, but only if `subpattern` matches successfully.
  If `subpattern` itself produces bindings (rare/advanced), those may be appended after the subject.
* The handler parameter order corresponds exactly to the binding order
* Patterns that do not bind values do not affect handler signatures

> [!IMPORTANT]
> `lit()`, `lit<>()`, and `has<>` never introduce bindings by themselves.
> All bindings are introduced exclusively by `$()` or `bind()`.

```cpp
// Binding order examples
$()                           -> (subject)
$(lit(...))                   -> (subject)
$(has<&A::x, &A::y>())        -> (x, y)
```

**Design Rationale**:

Patternia deliberately separates *matching* from *binding*.
A pattern answers **"does this value match?"**
A binding answers **"what values become available to the handler?"**

This separation keeps control flow declarative and data flow explicit.

---

## III. Guard System

Guards are **declarative constraints attached to patterns**. They refine a match **after binding succeeds**, and before the handler is invoked.

A guard failure **does not terminate** matching; it simply makes the current case fail and the engine continues to the next `.when(...)`.

!!! warning
    `_0` and `rng(...)` are only valid when exactly one value is bound **now**.
    For multi-value patterns, use `arg<N>` or a lambda predicate.

---

### `[]` Guard Attachment

**Syntax**:

```cpp
pattern[guard]
```

**Evaluation Order**:

1. `pattern.match(subject)`
2. `pattern.bind(subject)` -> produces bound values (the handler inputs)
3. evaluate `guard` (against the bound values)
4. if guard passes -> invoke handler; otherwise try next case

**Example**:

```cpp
match(x) | on(
  $()[_0 > 0] >> [](int v) {
    // 1) $() matches
    // 2) $() binds v
    // 3) guard (_0 > 0) is evaluated
    // 4) handler runs only if guard passes
  },
  __ >> [] {}
);
```

Type patterns can be guarded as well, as long as they bind:

```cpp
match(v) | on(
  $(is<std::string>())[_0 != ""] >> [](const std::string &s) {
    /* guarded alternative */
  },
  __ >> [] {}
);
```

**Guard composition** is supported:

* `&&` (logical AND)
* `||` (logical OR)

---

### Single-value Guards: `_0` and `rng(...)`

Single-value guards are built with the global placeholder:

```cpp
inline constexpr arg_t<0> _0{};
```

`_0` is **not** a runtime value. Expressions like `_0 > 0` **construct a predicate object**.

#### Supported operators for `_0`

```cpp
_0 >  rhs
_0 <  rhs
_0 >= rhs
_0 <= rhs
_0 == rhs
_0 != rhs
```

#### Range helper: `rng(lo, hi, mode)`

Use `rng(...)` for interval constraints (it returns a predicate usable inside `[]`).

```cpp
rng(lo, hi)                 // [lo, hi]
rng(lo, hi, open)           // (lo, hi)
rng(lo, hi, open_closed)    // (lo, hi]
rng(lo, hi, closed_open)    // [lo, hi)
```

**Examples**:

```cpp
$()[_0 > 0 && _0 < 10]
$()[rng(0, 10, closed_open)]   // [0, 10)
```

Anything beyond these relational/range constraints should be expressed as a **custom predicate** (lambda), not as additional operator DSL.

---

### Multi-value Guards: `arg<N>` Expressions

For guards that relate **multiple bound values**, use `arg<N>` to reference the *N-th bound value* (0-based):

```cpp
template <std::size_t I>
inline constexpr arg_t<I> arg{};
```

**Key rule**: `arg<N>` indices correspond **exactly** to the pattern’s binding order (and therefore the handler’s parameter order).

#### Supported operators for multi-value expressions

Multi-value guard expressions support:

* comparisons: `== != < > <= >=`
* arithmetic: `+ - * / %`
* plus logical composition via `&&` and `||`

There is **no `rng(...)`** for `arg<N>` expressions in the current design.

**Examples**:

```cpp
// x + y == 0
match(p) | on(
  $(has<&Point::x, &Point::y>())[arg<0> + arg<1> == 0] >>
    [](int x, int y) { /* ... */ },
  __ >> [] {}
);
```

```cpp
// Protocol-like constraint over multiple fields
match(pkt) | on(
  $(has<&Packet::type, &Packet::length>())[arg<0> == 0x01 && arg<1> == 0] >>
    [](auto type, auto len) { /* ... */ },
  __ >> [] {}
);
```

#### Compile-time boundary checking

Using an out-of-range `arg<N>` is **ill-formed** and diagnosed at compile time (the library checks the maximum referenced index against the number of bound values):

```cpp
$(has<&Point::x>())[arg<1> > 0]
// ill-formed: arg<1> out of range (only one bound value)
```

---

### Custom Predicates (Recommended for "Domain Logic")

A critical constraint of Patternia's guard DSL is that `arg<N>` is an **expression-template placeholder**, not "the real field type" in a way that enables arbitrary member access in the DSL.

So this is **not supported** as a guard DSL expression:

```cpp
$(has<&Message::payload>())[arg<0>.size() > 0] // do not do this
```

When you need container queries, method calls, non-trivial computations, or any domain-specific logic, write a **lambda predicate** and pass it to `[]`.

**Examples**:

```cpp
auto non_empty = [](auto const& payload) {
  return !payload.empty();
};

match(msg) | on(
  $(has<&Message::payload>())[non_empty] >> [](auto const& payload) {
    // payload is guaranteed non-empty here
  },
  __ >> [] {}
);
```

```cpp
auto sum_is_even = [](int a, int b) {
  return ((a + b) % 2) == 0;
};

match(p) | on(
  $(has<&Point::x, &Point::y>())[sum_is_even] >> [](int x, int y) { /* ... */ },
  __ >> [] {}
);
```

---

## IV. Structural Matching

Structural matching allows Patternia to reason about the *shape* of objects at compile time,
independently of control flow and value-level logic.

Rather than destructuring by position, Patternia describes structure through **explicit member selection**.

---

### `has<&T::member...>`

**Role**: Structural constraint and decomposition primitive.

**Syntax**:
```cpp
template <auto... Ms>
constexpr auto has();
```

Each template argument must be a pointer to a non-static data member.

---

### Semantics

`has<Ms...>` declares a **compile-time structural requirement**:

* The matched type must contain all listed data members
* The order of members is **not positional**
* No values are accessed unless used within `bind()`

This makes `has<>` a *structural predicate*, not a value pattern.

---

### Core Characteristics

* Declarative structural constraints without control flow
* No implicit value access or extraction
* Independent of member layout or declaration order
* Compile-time validation of member pointers

---

### Basic Examples

```cpp
struct Point {
  int x;
  int y;
};

struct Packet {
  std::uint8_t type;
  std::uint16_t length;
  std::vector<std::uint8_t> data;
};

// Structural checks
has<&Point::x, &Point::y>
has<&Packet::type, &Packet::length>
```

These patterns assert that the matched value exposes the listed members.
They do **not** bind or extract any values by themselves.

---

### Integration with `$()`

When combined with `$()`, `has<>` enables **explicit structural extraction**.

```cpp
match(p) | on(
  $(has<&Point::x, &Point::y>()) >> [](int x, int y) {
    return fmt("({}, {})", x, y);
  },
  __ >> [] {}
);
```

* `has<>` performs the structural check
* `$()` extracts the selected member values
* The handler receives the extracted values in the order listed

---

### Partial Structural Matching

Structural matching is **selective by design**.

To bind only a subset of members, list only those members:

```cpp
match(p) | on(
  $(has<&Point::x>()) >> [](int x) {
    return fmt("x = {}", x);
  },
  __ >> [] {}
);
```

No placeholder or positional skipping is required.
Unlisted members are simply ignored.

---

### Design Rationale

Patternia intentionally avoids positional or placeholder-based structural patterns.

* There is no notion of "ignored slots"
* There is no dependency on member order
* There is no implicit reflection

Instead:

* Structure is described by **explicit member pointers**
* Decomposition is driven by **what you list**, not what you omit
* This aligns with C++’s existing type system and avoids reflection-like assumptions

---

### Standalone vs Bound Usage

```cpp
has<&T::a, &T::b>        // Structural constraint only
bind(has<&T::a, &T::b>) // Structural constraint + value extraction
```

This separation ensures that:

* Structural reasoning remains declarative
* Data flow remains explicit
* Control flow remains predictable

---

### Compile-time Guarantees

* All member pointers are validated at compile time
* Invalid or non-existent members are diagnosed immediately
* No runtime reflection or metadata is required

---

### Conceptual Analogy

`has<>` is conceptually similar to Rust’s structural patterns:

```rust
Point { x, y }
```

—but adapted to C++’s lack of native reflection by using explicit member pointers.

It expresses **what structure is required**, not **how to traverse it**.

---


## V. Namespace Structure

Patternia employs a layered namespace architecture:

```
ptn/                                    // Root namespace
├── core/                              // Core matching engine
│   ├── engine/                        // Matching engine implementation
│   ├── dsl/                           // DSL operators
│   └── common/                        // Common utilities and traits
├── pat/                              // Pattern definitions
│   ├── base/                          // Pattern base classes
│   ├── lit.hpp                        // Literal pattern implementation
│   ├── bind.hpp                       // Binding pattern implementation
│   ├── wildcard.hpp                   // Wildcard pattern
│   ├── structural.hpp                 // Structural pattern
│   └── modifiers/                     // Pattern modifiers
│       └── guard.hpp                  // Guard system
└── meta/                             // Metaprogramming tools
    ├── base/                         // Base traits
    ├── dsa/                          // Data structures and algorithms
    └── query/                        // Query utilities
```

### Primary Namespace Aliases

For user convenience, Patternia provides all pattern functions directly in the `ptn` namespace:

```cpp
namespace ptn {
  // Core matching
  using ptn::core::engine::match;
  
  // Pattern primitives
  using ptn::pat::lit;
  using ptn::pat::lit_ci;
  using ptn::pat::bind;
  using ptn::pat::_;

  // Guard system
  using ptn::pat::mod::_0;
  using ptn::pat::mod::_1;
  using ptn::pat::mod::_2;
  using ptn::pat::mod::_3;
  using ptn::pat::mod::arg;
  using ptn::pat::mod::rng;

  // Structural patterns
  using ptn::pat::has;

  // Type patterns (variable templates)
  template <typename T>
  inline constexpr auto is = ptn::pat::is<T>;
  
  template <std::size_t I>
  inline constexpr auto alt = ptn::pat::alt<I>;
}
```

---

This API reference focuses on Patternia's core design philosophy and essential APIs, providing readers with an understanding of Patternia's language composition rather than merely a function list. This organization facilitates better comprehension of Patternia's mental model and usage patterns.









