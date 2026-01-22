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
match<AsType>(subject)
```

**Core Characteristics**:

* `subject` is the value being matched, not an implicitly captured variable
* `match()` returns a builder object; no matching is performed immediately
* Cases are evaluated sequentially using **first-match semantics**

**Basic Usage**:

```cpp
match(x)
  .when(lit(1) >> [] { return "one"; })
  .otherwise("default");
```

**Type Control**:

```cpp
// Automatic type deduction
int x = 42;
match(x);   // subject matched as int

// Explicit matching view
double d =3.14;
match<int>(d);  // subject is matched as int
```

Specifying `AsType` explicitly forces the subject to be viewed as that type during pattern evaluation.

---

### `match(subject, cases(...))`

**Role**: Compact syntax for simple matching scenarios.

**Syntax**:
```cpp
match(subject, cases(case1, case2, ...))
```

**Key Characteristics**:

* Provides a concise alternative to the standard `.when()` chain syntax
* **No guards/predicates**: Cannot be used with guard expressions or predicates
* Supports both pattern fallback (`__`) and match fallback patterns
* Cases are evaluated sequentially using **first-match semantics**
* **Requires `.end()`** to trigger evaluation when using `__` pattern

**Basic Usage**:

```cpp
match(x, cases(
  lit(1) >> [] { std::cout << "one\n"; },
  lit(2) >> [] { std::cout << "two\n"; },
  __    >> [] { std::cout << "other\n"; }
)).end();
```

**Limitations**:

- Cannot use guard expressions `[]` with tuple syntax
- Cannot use predicate-based guards
- Best suited for simple literal and wildcard matching
- For complex matching with guards, use the standard DSL syntax
- **Must use `.end()` when using `__` pattern to trigger evaluation**

**Design Intent**:
This syntax is optimized for straightforward value discrimination where the full power of the DSL (guards, complex patterns) is not needed.

---

### `.when(pattern >> handler)`

**Role**: Primary case-definition primitive.

**Syntax**:

```cpp
.when(pattern >> handler)
```

**Key Characteristics**:

* `pattern >> handler` forms a *case expression*
* The handler’s parameter list is determined entirely by the pattern’s binding behavior
* `.when()` only registers a branch; it does not trigger execution

**Usage Examples**:

```cpp
// Handler without bindings
.when(lit(1) >> [] { return "one"; })   // No parameters
.when(lit(2) >> 42)                     // Constant value handler

// Handler with bindings
.when(bind() >> [](int v) {
  return v * 2;
})

// Structural bindings
.when(
  bind(has<&Point::x, &Point::y>()) >>
  [](int x, int y) {
    return x + y;
  }
)
```

**Handler Forms**:

* **Value Handler** — `pattern >> value`
  Returns a fixed value. Semantically equivalent to a zero-argument function returning a constant.

* **Function Handler** — `pattern >> callable`
  Receives the values produced by the pattern’s bindings.

---

### `.otherwise(...)` and `.end()`

These terminal operations define how a match expression is finalized.
The key distinction between them is the use of the `__` pattern.

---

#### `.otherwise(...)`

**Role**: Match fallback that works in all scenarios.

**Purpose**: Provides a fallback when no prior `when` clause matches successfully.

**Characteristics**:

* Can be used regardless of whether `__` pattern is present
* Acts as a defensive default, not as a regular case
* Evaluated only if no pattern has already matched

```cpp
auto r = match(x)
  .when(lit(1) >> 10)
  .when(lit(2) >> 20)
  .otherwise(0);
```

**Execution Semantics**:

```cpp
auto result = match(42)
  .when(lit(1) >> "one")      // No match
  .when(lit(2) >> "two")      // No match
  .otherwise("default");          // Executed as fallback
```

---

#### `.end()`

**Purpose**: Required when using the `__` pattern fallback.

**Characteristics**:

* **Must** be used when `__` pattern is present in the match
* Can be used with or without return values (not limited to `void`)
* Not required when only using `.otherwise()`

```cpp
match(x)
  .when(lit(1) >> [] { std::cout << "one\n"; })
  .when(lit(2) >> [] { std::cout << "two\n"; })
  .when(__   >> [] { std::cout << "other\n"; })  // pattern fallback
  .end();  // Required for __ to work
```

**Critical Rule**: If `__` pattern is used without `.end()`, the `__` case will **not trigger**.

**Design Philosophy**:
The distinction between `.otherwise()` and `.end()` is driven by the `__` pattern's requirement for match inference.

---

### Comparison Summary

| Feature                  | `.otherwise()` | `.end()` |
| ------------------------ | -------------- | ---------- |
| Key Requirement          | None          | Required for `__` pattern |
| Return Type             | Handler-determined | Handler-determined |
| Fallback Type           | Match fallback  | Pattern fallback (`__`) |
| Usage Constraint         | Works in all scenarios | Must be used with `__` |
| Typical Use Cases       | Value computation, defensive fallback | Exhaustive matching with `__` |

---

## II. Pattern Primitives

Pattern primitives constitute the core vocabulary of Patternia’s pattern language.  
They describe *what* is matched, *what* is ignored, and *what* is explicitly bound.

---

### `lit(value)`

**Role**: Fundamental value pattern for exact matching.

**Syntax**:
```cpp
template <typename V>
constexpr auto lit(V &&v);

template <typename V>
constexpr auto lit_ci(V &&v);  // Case-insensitive matching
```

**Semantics**:

* Matches the subject using `operator==`
* Produces no bindings
* Intended for value-based discrimination

**Examples**:

```cpp
.when(lit(Status::Running) >> ...)   // Enum matching
.when(lit(42) >> ...)                // Integer literal
.when(lit_ci("hello") >> ...)        // Case-insensitive string match
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
  .otherwise("fallback");              // Expression-level fallback
```

**Key Distinction**:

* `__` — a pattern that participates in matching and ordering
* `.otherwise()` — a match-level fallback executed only if no pattern matches

---

### `bind()` (Binding Pattern)

**Role**: Explicit value binding primitive.

`bind()` is the *only* mechanism in Patternia that introduces bindings into a match.

**Syntax**:

```cpp
constexpr auto bind();                    // Binds the entire subject

template <typename SubPattern>
constexpr auto bind(SubPattern &&sub);    // Binds subject conditionally under a subpattern constraint
```

**Core Principles**:

1. All bindings are explicit
2. No pattern introduces bindings implicitly
3. Binding behavior fully determines handler parameter lists

---

#### Binding the Entire Subject

```cpp
.when(bind() >> [](auto v) {
  return "captured: " + std::to_string(v);
})
```

* The entire subject is bound as a single value
* The handler receives exactly one parameter

---

#### Conditional Binding with `bind(subpattern)`

```cpp
.when(
  bind(lit(Status::Running)) >>
  [](Status s) {
    return fmt("status = {}", static_cast<int>(s));
  }
)
````

* `bind(subpattern)` introduces a binding **only if the subpattern matches**
* The bound value is always the **entire subject**
* The subpattern itself does **not** introduce bindings unless explicitly designed to do so

This form is useful when:

* a value must be both **filtered** and **captured**
* the captured value needs to be reused by the handler
* the constraint is semantic rather than structural


#### Structural Binding with `has<>`

```cpp
struct Point { int x, int y; };

.when(
  bind(has<&Point::x, &Point::y>()) >>
  [](int x, int y) {
    return fmt("({}, {})", x, y);
  }
)
```

* `has<>` describes the required structure
* Only the listed members are bound
* No access to unlisted members is provided

Partial binding is expressed by listing only the desired members:

```cpp
.when(
  bind(has<&Point::x>()) >>
  [](int x) {
    return fmt("x = {}", x);
  }
)
```

#### Binding Semantics

* `bind()` always binds the *entire subject*
* `bind(subpattern)` binds the subject, but only if `subpattern` matches successfully.
  If `subpattern` itself produces bindings (rare/advanced), those may be appended after the subject.
* The handler parameter order corresponds exactly to the binding order
* Patterns that do not bind values do not affect handler signatures

> [!IMPORTANT]
> `lit()` and `has<>` never introduce bindings by themselves.
> All bindings are introduced exclusively by `bind(...)`.

```cpp
// Binding order example
bind()                        -> (subject)
bind(lit(...))                -> (subject)
bind(has<&A::x, &A::y>())     -> (x, y) // because has() is used under bind(), extraction is defined by bind(...)
```

**Design Rationale**:

Patternia deliberately separates *matching* from *binding*.
A pattern answers **“does this value match?”**
A binding answers **“what values become available to the handler?”**

This separation keeps control flow declarative and data flow explicit.

---

### `type::is<T>()` and `type::as<T>()` (Variant Type Matching)

**Role**: Match `std::variant` alternatives by type, with optional explicit binding.

**Syntax**:
```cpp
type::is<T>()                 // type-only match
type::is<T>(subpattern)       // apply subpattern to the alternative

type::as<T>()                 // explicit binding sugar for is<T>(bind())
type::as<T>(subpattern)       // explicit binding sugar for is<T>(bind(subpattern))
```

**Shorthand**:
```cpp
is<T>()   // alias of type::is<T>()
as<T>()   // alias of type::as<T>()
```

**Key Properties**:

* Works on `std::variant` subjects only
* `type::is<T>()` does not bind values
* `type::as<T>()` is an explicit binding shortcut; it does not introduce implicit binding
* Alternative type `T` must appear exactly once in the variant

**Examples**:

```cpp
match(v)
  .when(type::is<int>() >> [] { /* type-only */ })
  .when(type::as<std::string>() >> [](const std::string &s) { /* bound */ })
  .when(type::is<Point>(bind(has<&Point::x, &Point::y>())) >>
        [](int x, int y) { /* structural bind */ })
  .otherwise([] {});
```

**Design Note**:
`type::as<T>()` preserves the "explicit bind" rule by being a named shortcut for
`bind()`, not an implicit binding mechanism.


---

## III. Guard System

Guards are **declarative constraints attached to patterns**. They refine a match **after binding succeeds**, and before the handler is invoked.

A guard failure **does not terminate** matching; it simply makes the current case fail and the engine continues to the next `.when(...)`.

!!! warning
    `_` and `rng(...)` are only valid when exactly one value is bound **now**.
    For multi-value patterns, use `arg<N>` or a lambda predicate.

---

### `[]` Guard Attachment

**Syntax**:

```cpp
pattern[guard]
```

**Evaluation Order**:

1. `pattern.match(subject)`
2. `pattern.bind(subject)` → produces bound values (the handler inputs)
3. evaluate `guard` (against the bound values)
4. if guard passes → invoke handler; otherwise try next case

**Example**:

```cpp
match(x)
  .when(
    bind()[_ > 0] >>
    [](int v) {
      // 1) bind() matches
      // 2) bind() binds v
      // 3) guard (_ > 0) is evaluated
      // 4) handler runs only if guard passes
    }
  )
  .otherwise([] {});
```

**Guard composition** is supported:

* `&&` (logical AND)
* `||` (logical OR)

---

### Single-value Guards: `_` and `rng(...)`

Single-value guards are built with the global placeholder:

```cpp
inline constexpr placeholder_t _;
```

`_` is **not** a runtime value. Expressions like `_ > 0` **construct a predicate object**.

#### Supported operators for `_`

```cpp
_ >  rhs
_ <  rhs
_ >= rhs
_ <= rhs
_ == rhs
_ != rhs
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
bind()[_ > 0 && _ < 10]
bind()[rng(0, 10, closed_open)]   // [0, 10)
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
match(p)
  .when(
    bind(has<&Point::x, &Point::y>())[arg<0> + arg<1> == 0] >>
    [](int x, int y) { /* ... */ }
  )
  .otherwise([] {});
```

```cpp
// Protocol-like constraint over multiple fields
match(pkt)
  .when(
    bind(has<&Packet::type, &Packet::length>())
      [arg<0> == 0x01 && arg<1> == 0] >>
    [](auto type, auto len) { /* ... */ }
  )
  .otherwise([] {});
```

#### Compile-time boundary checking

Using an out-of-range `arg<N>` is **ill-formed** and diagnosed at compile time (the library checks the maximum referenced index against the number of bound values):

```cpp
bind(has<&Point::x>())[arg<1> > 0]
// ❌ ill-formed: arg<1> out of range (only one bound value)
```

---

### Custom Predicates (Recommended for “Domain Logic”)

A critical constraint of Patternia’s guard DSL is that `arg<N>` is an **expression-template placeholder**, not “the real field type” in a way that enables arbitrary member access in the DSL.

So this is **not supported** as a guard DSL expression:

```cpp
bind(has<&Message::payload>())[arg<0>.size() > 0] // ❌ do not do this
```

When you need container queries, method calls, non-trivial computations, or any domain-specific logic, write a **lambda predicate** and pass it to `[]`.

**Examples**:

```cpp
auto non_empty = [](auto const& payload) {
  return !payload.empty();
};

match(msg)
  .when(
    bind(has<&Message::payload>())[non_empty] >>
    [](auto const& payload) {
      // payload is guaranteed non-empty here
    }
  )
  .otherwise([] {});
```

```cpp
auto sum_is_even = [](int a, int b) {
  return ((a + b) % 2) == 0;
};

match(p)
  .when(
    bind(has<&Point::x, &Point::y>())[sum_is_even] >>
    [](int x, int y) { /* ... */ }
  )
  .otherwise([] {});
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

### Integration with `bind()`

When combined with `bind()`, `has<>` enables **explicit structural extraction**.

```cpp
match(p)
  .when(
    bind(has<&Point::x, &Point::y>()) >>
    [](int x, int y) {
      return fmt("({}, {})", x, y);
    }
  )
  .otherwise([] {});
```

* `has<>` performs the structural check
* `bind()` extracts the selected member values
* The handler receives the extracted values in the order listed

---

### Partial Structural Matching

Structural matching is **selective by design**.

To bind only a subset of members, list only those members:

```cpp
match(p)
  .when(
    bind(has<&Point::x>()) >>
    [](int x) {
      return fmt("x = {}", x);
    }
  )
  .otherwise([] {});
```

No placeholder or positional skipping is required.
Unlisted members are simply ignored.

---

### Design Rationale

Patternia intentionally avoids positional or placeholder-based structural patterns.

* There is no notion of “ignored slots”
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
│   ├── dsl/                          // DSL operators
│   └── common/                       // Common utilities and traits
├── pat/                              // Pattern definitions
│   ├── base/                         // Pattern base classes
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
  using ptn::pat::__;

  // Guard system
  using ptn::pat::mod::_;
  using ptn::pat::mod::arg;
  using ptn::pat::mod::rng;

  // Structural patterns
  using ptn::pat::has;

  // Type patterns
  namespace type = ptn::pat::type;
  using ptn::pat::type::is;
  using ptn::pat::type::as;
}
```

---

This API reference focuses on Patternia's core design philosophy and essential APIs, providing readers with an understanding of Patternia's language composition rather than merely a function list. This organization facilitates better comprehension of Patternia's mental model and usage patterns.
