# Patternia API Reference

## Namespace Structure Overview

Patternia adopts a layered namespace design, primarily containing the following namespaces:

```
ptn/                                    // Root namespace
├── core/                              // Core matching engine
│   ├── engine/                        // Matching engine implementation
│   ├── dsl/                          // DSL operators
│   └── common/                       // Common utilities and traits
├── pat/                              // Pattern definitions
│   ├── base/                         // Pattern base classes
│   ├── lit.hpp                        // Literal pattern implementation
│   └── bind.hpp                       // Binding pattern implementation
└── meta/                             // Meta programming tools
    ├── base/                         // Base traits
    ├── dsa/                          // Data structures and algorithms
    └── query/                        // Query utilities
```

### Main Namespace Aliases

For user convenience, Patternia provides all pattern functions directly in the `ptn` namespace:

```cpp
namespace ptn {
  // lit, lit_ci, bind are available directly
}
```

---

## Core Matching Engine API

### Main Entry Functions

#### `ptn::match(value)`

Type-deduced matching entry point that automatically deduces the value type.

```cpp
template <typename T>
constexpr auto match(T &&value);
```

**Parameters:**
- `value` - Value to match (forwarding reference)

**Returns:** `match_builder` object for chained calls to `.when()` and `.otherwise()`

**Example:**
```cpp
int x = 42;
auto result = match(x)
    .when(lit(42) >> "answer")
    .otherwise("other");
```

#### `ptn::match<U>(value)`

Explicit type-specified matching entry point that forces conversion to target type.

```cpp
template <typename U, typename T>
constexpr auto match(T &&value);
```

**Parameters:**
- `U` - Target type
- `value` - Value to match

**Constraints:** Type U must be constructible from value

**Example:**
```cpp
double d = 3.14;
auto result = match<int>(d)  // Force convert to int
    .when(lit(3) >> "three")
    .otherwise("other");
```

### Match Builder API

#### `match_builder::when(pattern >> handler)`

Add a matching branch.

**Syntax:**
```cpp
.when(pattern >> handler)
```

**Parameters:**
- `pattern` - Any pattern object
- `handler` - Handler function or value

**Returns:** New `match_builder` object (supports chained calls)

#### `match_builder::otherwise(handler)`

Add a default branch.

**Syntax:**
```cpp
.otherwise(handler)
```

**Parameters:**
- `handler` - Default handler function or value

**Returns:** Match result

---

## Value Pattern API

!!! note
    **`ptn::pat::lit`**, **`ptn::pat::lit_ci`**, and **`ptn::pat::bind`** are available directly in `ptn` namespace

### Literal Patterns

#### `ptn::lit(value)`

Create exact match pattern.

```cpp
template <typename V>
constexpr auto lit(V &&v);
```

**Purpose:** Create a pattern that exactly matches the given value

**Supported Types:**
- Arithmetic types (int, double, float, etc.)
- Enum types
- Strings (std::string, std::string_view, const char*)
- User-defined types (must support operator==)

**Examples:**
```cpp
// Integer matching
match(42).when(lit(42) >> "answer");

// String matching
match("hello").when(lit("hello") >> "greeting");

// Enum matching
enum class Color { Red, Green, Blue };
Color c = Color::Red;
match(c).when(lit(Color::Red) >> "red");
```

#### `ptn::lit_ci(value)`

Create case-insensitive string matching pattern.

```cpp
template <typename V>
constexpr auto lit_ci(V &&v);
```

**Purpose:** Create a case-insensitive string matching pattern (ASCII only)

**Example:**
```cpp
std::string s = "HELLO";
match(s).when(lit_ci("hello") >> "case-insensitive-match");
```

### Binding Patterns

#### `ptn::bind()`

Create a pattern that captures the entire subject value.

```cpp
constexpr auto bind();
```

**Purpose:** Create a pattern that always matches and captures the subject as a tuple

**Returns:** `binding_pattern` that binds the subject value

**Examples:**
```cpp
// Capture any value
match(42).when(bind() >> [](int v) { 
    return "captured: " + std::to_string(v);
});

// Fallback capture
enum Status { Pending, Running };
match(Status::Running)
    .when(lit(Status::Pending) >> "pending")
    .when(bind() >> [](int v) { return "status: " + std::to_string(v); });
```

#### `ptn::bind(subpattern)`

Create a pattern that first matches with subpattern, then captures the subject.

```cpp
template <typename SubPattern>
constexpr auto bind(SubPattern &&subpattern);
```

**Purpose:** Create a pattern that combines subpattern matching with subject capturing

**Parameters:**
- `subpattern` - Pattern to use for matching

**Returns:** `binding_as_pattern` that binds both subject and subpattern bindings

**Examples:**
```cpp
// Capture after matching specific value
enum Status { Pending, Running, Completed };
match(Status::Running)
    .when(bind(lit(Status::Running)) >> [](int whole) {
        std::cout << "Captured enum value: " << whole << "\n";
        return "running";
    });

// Capture with string matching
std::string cmd = "START";
match(cmd)
    .when(bind(lit_ci("start")) >> [](std::string command) {
        return "processing: " + command;
    });
```

**Binding Behavior:**
- `bind()` alone binds the subject as `std::tuple<Subject>`
- `bind(subpattern)` binds `std::tuple<Subject, SubPatternBindings...>`
- Handler functions receive the bound values as parameters

---

## DSL Operators API

### Pattern-Handler Binding Operators

#### `pattern >> handler` - Pattern binding

Bind a pattern with a handler function or value.

**Two modes:**

1. **Value handler mode:**
   ```cpp
   pattern >> value
   ```
   Automatically creates a handler that returns a fixed value

2. **Function handler mode:**
   ```cpp
   pattern >> function
   ```
   Directly uses the provided function as the handler

**Examples:**
```cpp
// Value handler
match(42).when(lit(42) >> "answer");  // Returns string "answer"

// Function handler
match(42).when(lit(42) >> []{ 
    return "answer";
});  // Returns string "answer"

// Capturing handler with bind()
match(42).when(bind() >> [](int value) { 
    return "captured: " + std::to_string(value);
});

// Capturing handler with subpattern
enum Status { Running };
match(Status::Running)
    .when(bind(lit(Status::Running)) >> [](int whole) {
        return "status: " + std::to_string(whole);
    });
```

---

## Binding System API

### Binding Traits

Patternia uses a type trait system to determine what values each pattern binds.

#### `binding_args<Pattern, Subject>`

Trait that defines the binding result type for a pattern-subject pair.

```cpp
template <typename Pattern, typename Subject>
struct binding_args {
    using type = std::tuple<>; // default implementation
};
```

**Specializations:**
- `binding_args<binding_pattern, Subject>` → `std::tuple<Subject>`
- `binding_args<binding_as_pattern<Tag, SubPattern>, Subject>` → concatenated tuple

#### `binding_args_t<Pattern, Subject>`

Convenience alias for binding arguments type.

```cpp
template <typename Pattern, typename Subject>
using binding_args_t = typename binding_args<Pattern, Subject>::type;
```

---

## API Feature Summary

### Type Safety
- All pattern matching is compile-time type safe
- Support for heterogeneous comparisons (comparisons between different types)
- Automatic type deduction and explicit type specification
- Strong typing for binding values

### Performance Optimization
- Zero runtime overhead abstractions
- Compile-time pattern combination optimization
- Support for constant expression evaluation
- No RTTI or virtual dispatch

### Extensibility
- CRTP-based pattern base classes
- Easy to add custom pattern types
- Support for custom comparators and predicates
- Modular binding trait system

### Expressiveness
- Rich built-in pattern types (literal, binding, case-insensitive)
- Intuitive DSL syntax with `>>` operator
- Support for complex pattern combinations
- Value capturing and binding capabilities

### Compatibility
- Support for C++17 and C++20
- Modular header file design
- Configurable feature switches
- Header-only integration

This complete API reference covers all public interfaces of the Patternia library, including internal and external APIs, and provides comprehensive example code. You can use this documentation to write GitHub Pages documentation and README files.
