# Patternia API Reference

## Introduction

Patternia provides a comprehensive pattern matching system for C++ with a focus on type safety, performance, and expressiveness. This document outlines the core APIs that constitute Patternia's pattern matching language.

---

## I. Match DSL Core Framework

The Match DSL forms the foundation of Patternia and determines how users understand the system. These APIs have the highest priority for comprehension.

---

### `match(subject)`

**Role**: DSL entry point and evaluation context for the entire system.

**Syntax**:
```cpp
template <typename T>
constexpr auto match(T &&value);

template <typename U, typename T>
constexpr auto match(T &&value);
```

**Core Characteristics**:

* `subject` represents the value being matched, not implicit capture
* `match()` returns a builder object, not immediate execution
* Cases are evaluated with first-match semantics in sequential order

**Basic Usage**:
```cpp
match(x)
  .when(lit(1) >> [] { return "one"; })
  .otherwise("default");
```

**Type Deduction**:
```cpp
// Automatic type deduction
int x = 42;
match(x)  // deduced as int

// Explicit type specification
double d = 3.14;
match<int>(d)  // forces conversion to int
```

---

### `.when(pattern >> handler)`

**Role**: Primary case definition primitive.

**Syntax**:
```cpp
.when(pattern >> handler)
```

**Key Characteristics**:

* `pattern >> handler` constitutes a case expression
* Handler parameters are determined entirely by the pattern's binding behavior
* `when` registers branches without executing them

**Usage Examples**:

```cpp
// Handler without bindings
.when(lit(1) >> [] { return "one"; })           // No parameters
.when(lit(1) >> 42)                              // Value handler

// Handler with bindings
.when(bind() >> [](int v) { return v * 2; })     // Single binding
.when(bind(lit(1)) >> [](int subject, int) { })  // Multiple bindings
```

**Handler Types**:
- **Value Handler**: `pattern >> value` - Returns a fixed value
- **Function Handler**: `pattern >> lambda` - Receives bound parameters

---

### `.otherwise(...)` and `.end()`

These terminal methods represent a distinctive design aspect of Patternia that requires careful explanation.

---

#### `.otherwise(...)`

**Trigger Condition**: Executed when no when clauses match successfully.

**Purpose**: Used for expression-style matching.

**Characteristics**:
- Must return a type consistent with other branches
- Provides defensive fallback behavior

```cpp
auto r = match(x)
  .when(lit(1) >> 10)
  .otherwise(0);  // Default value
```

**Execution Semantics**:
```cpp
auto result = match(42)
    .when(lit(1) >> "one")      // No match
    .when(lit(2) >> "two")      // No match  
    .when(__ >> "other")        // Pattern matches, returns "other"
    .otherwise("default");       // Not executed: pattern already matched
```

---

#### `.end()`

**Purpose**: Used for statement-style matching.

**Characteristics**:
- Requires all handlers to return void
- No fallback execution
- Compile-time verification of exhaustive case coverage

```cpp
match(x)
  .when(lit(1) >> [] { log("one"); })
  .when(__   >> [] { log("other"); })
  .end();  // All cases must be covered
```

**Design Philosophy**: This represents a key divergence from Rust/Scala pattern matching systems.

**Comparison Summary**:

| Feature | `.otherwise()` | `.end()` |
|---------|----------------|----------|
| Return Type | Handler-determined | `void` |
| Use Case | Expression | Statement |
| Safety | Always has fallback | Compile-time exhaustiveness check |
| Typical Scenario | Value computation | Side effects execution |

---

## II. Pattern Primitives

These constitute the vocabulary of Patternia's pattern language.

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
- Uses `operator==` for precise matching
- Produces no bindings
- Suitable for enums, integers, literals, and constexpr values

**Examples**:
```cpp
.when(lit(Status::Running) >> ...)    // Enum matching
.when(lit(42) >> ...)                  // Integer matching
.when(lit_ci("hello") >> ...)         // Case-insensitive string matching
```

**Supported Types**:
- Arithmetic types (int, double, float)
- Enumeration types
- Strings (std::string, std::string_view, const char*)
- User-defined types (must support operator==)

---

### `__` (Wildcard)

**Role**: Fallback pattern for matching any value without binding.

**Syntax**:
```cpp
inline constexpr detail::wildcard_t __;
```

**Key Properties**:

* Matches any value
* No binding behavior
* Distinct semantics from `.otherwise()`

```cpp
.when(__ >> [] { /* Cannot access matched value */ })
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
- `__`: Pattern-level wildcard, participates in matching process
- `.otherwise()`: Expression-level final safety net

---

### `bind()` (Binding Pattern)

**Role**: Soul API of Patternia for explicit value capture.

**Syntax**:
```cpp
constexpr auto bind();                    // Binds entire subject
template <typename SubPattern>
constexpr auto bind(SubPattern &&sub);    // Binds with subpattern matching
```

**Core Concepts**:

1. `bind()` is the exclusive mechanism for explicit binding introduction
2. No implicit binding occurs
3. Binding behavior determines handler parameter lists

**Basic Usage**:
```cpp
.when(bind() >> [](auto v) { 
    return "captured: " + std::to_string(v); 
})
```

**Structural Binding**:
```cpp
// Bind structure members
struct Point { int x, y; };

.when(bind(has<&Point::x, &Point::y>()) >> [](int subject, int x, int y) {
    return fmt("point({}, {})", x, y);
})

// Bind partial members, ignore others
.when(bind(has<&Point::x, _ign>()) >> [](int subject, int x) {
    return fmt("x={}", x);
})
```

**Composite Binding**:
```cpp
// Match subpattern first, then bind entire value
.when(bind(lit(Status::Running)) >> [](int whole_status) {
    return fmt("status code: {}", whole_status);
})
```

**Binding Order**:
- `bind(subpattern)` binding order: `(subject, subpattern_bindings...)`
- Handler parameter order must correspond to binding order

---

## III. Guard System

The Guard system represents a key differentiator for Patternia in the C++ ecosystem.

### `[]` Guard Syntax

**Role**: One of Patternia's most distinctive features.

**Syntax**:
```cpp
pattern[guard]
```

**Key Characteristics**:

* Guard is part of the pattern, not a handler precondition
* Guard executes after binding but before handler execution
* Guard failure continues to next case rather than terminating matching

**Basic Examples**:
```cpp
bind()[_ > 0 && _ < 10]  // Range check
bind()[_ % 2 == 0]       // Even number check
```

**Execution Order**:
```cpp
.when(bind()[_ > 0] >> [](int v) { 
    // Execution sequence:
    // 1. bind() matches (always succeeds)
    // 2. bind() binds (captures value to tuple)
    // 3. guard evaluation (_ > 0)
    // 4. handler execution (if guard passes)
})
```

---

### `_` (Placeholder)

**Role**: Predicate constructor for guard expressions.

**Key Properties**:

* `_` is not a value but a predicate constructor
* `_ > 0` constructs a predicate, not a boolean
* Can only be used within guard contexts

```cpp
[_ % 2 == 0]           // Even predicate
[_ > 0 && _ < 100]     // Range predicate
```

**Supported Operators**:
```cpp
_ > 5           // Greater than
_ < 10          // Less than
_ >= 0          // Greater than or equal
_ <= 100        // Less than or equal
_ == 42         // Equal to
_ != 0          // Not equal to
```

**Compound Predicates**:
```cpp
[_ > 0 && _ < 10]     // AND composition
[_ < 0 || _ > 100]    // OR composition
```

---

### `arg<N>`

**Role**: Core API for multi-value guard expressions.

**Syntax**:
```cpp
template <std::size_t I>
inline constexpr arg_t<I> arg{};
```

**Characteristics**:

* `arg<0>`, `arg<1>` reference binding results by index
* Corresponds one-to-one with handler parameter order
* Supports arithmetic, relational, and logical combinations

**Basic Examples**:
```cpp
bind(has<&Point::x, &Point::y>())[arg<0> + arg<1> == 0]  // x + y == 0
bind(has<&Point::x, &Point::y>())[arg<0> > arg<1>]       // x > y
```

**Complex Expressions**:
```cpp
bind(has<&Packet::type, &Packet::length>())[arg<0> == 0x01 && arg<1] == 0]
bind(has<&Packet::payload>())[arg<0>.size() > 0]
```

**Boundary Checking**: Compile-time validation that arg<N> index does not exceed binding count
```cpp
bind(has<&Point::x>())[arg<1> > 0]  // Compile error: only one bound value
```

---

### Lambda Guards

**Custom Predicate Support**:

```cpp
auto is_prime = [](int v) {
    if (v < 2) return false;
    for (int i = 2; i * i <= v; ++i)
        if (v % i == 0) return false;
    return true;
};

bind()[is_prime]  // Custom predicate
```

**Multi-parameter Lambda**:
```cpp
auto sum_is_even = [](int a, int b) { return (a + b) % 2 == 0; };

bind(has<&Point::x, &Point::y>())[sum_is_even]
```

---

## IV. Structural Matching

### `has<&T::member...>`

**Role**: Unique Patternia API for C++ structural decomposition.

**Syntax**:
```cpp
template <auto... Ms>
constexpr auto has();
```

**Semantics**: Compile-time structural constraint declaration.

**Core Characteristics**:
- Declarative structural constraints without value access
- Value extraction only occurs within `bind()` context
- Analogous to Rust's `{ x, y, .. }` syntax

**Basic Examples**:
```cpp
struct Point { int x, y; };
struct Packet { uint8_t type; uint16_t length; std::vector<uint8_t> data; };

// Structural checks
has<&Point::x, &Point::y>           // Checks for x, y members
has<&Packet::type, &Packet::length>  // Checks for type, length members
```

**Placeholder Support**:
```cpp
using ptn::pat::_ign;  // or using ptn::_ign;

has<&Point::x, _ign>    // Checks for x member, ignores others
has<&Point::x, nullptr> // Same as above, using nullptr
```

**Integration with bind()**:
```cpp
.when(bind(has<&Point::x, &Point::y>()) >> [](int subject, int x, int y) {
    // subject: entire Point
    // x: Point::x value
    // y: Point::y value
    return fmt("({}, {})", x, y);
})

// Ignore certain members
.when(bind(has<&Point::x, _ign>()) >> [](int subject, int x) {
    return fmt("x={}", x);
})
```

**Design Philosophy**:
- `has()` standalone: structural checking only, no value extraction
- `bind(has())`: structural checking + value extraction
- Compile-time validation of member pointer validity

---

## V. Composition and Extension

### `&&` / `||` (Guard Composition)

**Guard Logical Composition**:

```cpp
// AND composition
[_ > 0 && _ < 100]                    // Built-in operators
[(_ > 0) && (_ < 100)]               // Explicit composition (identical functionality)

// OR composition  
[_ < 0 || _ > 100]                    // Built-in operators
[(_ < 0) || (_ > 100)]               // Explicit composition (identical functionality)

// Complex composition
[(_ > 0 && _ < 10) || (_ > 90 && _ <= 100)]
```

**Multi-value Guard Composition**:
```cpp
bind(has<&Point::x, &Point::y>())[arg<0> > 0 && arg<1] < 0]  // x > 0 && y < 0
```

**Lambda Composition**:
```cpp
auto positive = [](int v) { return v > 0; };
auto small    = [](int v) { return v < 10; };

bind()[positive && small]  // Equivalent to [_ > 0 && _ < 10]
```

---

### Range Predicates

**Range Checking API**:

```cpp
using ptn::pat::mod::rng;

// Closed interval [0, 100]
rng(0, 100)

// Open interval (0, 100)  
rng(0, 100, open)

// Left-open right-closed (0, 100]
rng(0, 100, open_closed)

// Left-closed right-open [0, 100)
rng(0, 100, closed_open)
```

**Examples**:
```cpp
bind()[rng(0, 100)]        // 0 <= v <= 100
bind()[rng(0, 100, open)]  // 0 < v < 100
```

---

### Custom Predicate Guards

**Custom Predicates**:

```cpp
// Simple lambda
auto is_even = [](int v) { return v % 2 == 0; };
bind()[is_even]

// Stateful functor
class RangeChecker {
    int lo_, hi_;
public:
    RangeChecker(int lo, int hi) : lo_(lo), hi_(hi) {}
    bool operator()(int v) const { return lo_ <= v && v <= hi_; }
};

bind()[RangeChecker(0, 100)]
```

**Multi-parameter Predicates**:
```cpp
auto sum_equals = [](int a, int b, int target) { return a + b == target; };

bind(has<&Point::x, &Point::y>())[std::bind_front(sum_equals, 10)]
// Alternative with lambda
bind(has<&Point::x, &Point::y>())[[](int x, int y) { return x + y == 10; }]
```

---

## VI. Design Philosophy Summary

### Core Principles

1. **Explicit over Implicit**: All bindings are explicit with no implicit capture
2. **Composition over Inheritance**: Complex patterns built from primitive composition
3. **Type Safety**: Compile-time guarantee of type correctness for all operations
4. **Zero Overhead**: All abstractions resolved at compile-time with no runtime cost

### Language Comparison

| Feature | Patternia | Rust | Scala | C++17 switch |
|---------|-----------|------|-------|--------------|
| Type Safety | Yes | Yes | Yes | Yes |
| Pattern Composition | Yes | Yes | Yes | No |
| Guard System | Yes | Yes | Yes | No |
| Structural Binding | Yes | Yes | Yes | No |
| Zero Overhead | Yes | Yes | Yes | Yes |
| Compile-time Checking | Yes | Yes | Yes | Partial |

### Extensibility

Patternia's design enables:
- Custom pattern types
- Custom guard predicates
- Complex pattern composition
- Seamless integration with existing C++ code

---

## VIII. Namespace Structure

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
  using ptn::pat::_ign;
}
```

---

This API reference focuses on Patternia's core design philosophy and essential APIs, providing readers with an understanding of Patternia's language composition rather than merely a function list. This organization facilitates better comprehension of Patternia's mental model and usage patterns.
