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
│   └── value/                        // Value patterns
└── meta/                             // Meta programming tools
    ├── base/                         // Base traits
    ├── dsa/                          // Data structures and algorithms
    └── query/                        // Query utilities
```

### Main Namespace Aliases

For user convenience, Patternia provides the following namespace aliases:

```cpp
namespace ptn {
  namespace value = pat::value;  // Value pattern alias
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
    **`ptn::pat::value::lit` == `ptn::lit`** 

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

```

---

## API Feature Summary

### Type Safety
- All pattern matching is compile-time type safe
- Support for heterogeneous comparisons (comparisons between different types)
- Automatic type deduction and explicit type specification

### Performance Optimization
- Zero runtime overhead abstractions
- Compile-time pattern combination optimization
- Support for constant expression evaluation

### Extensibility
- CRTP-based pattern base classes
- Easy to add custom pattern types
- Support for custom comparators and predicates

### Expressiveness
- Rich built-in pattern types
- Intuitive DSL syntax
- Support for complex pattern combinations

### Compatibility
- Support for C++17 and C++20
- Modular header file design
- Configurable feature switches

This complete API reference covers all public interfaces of the Patternia library, including internal and external APIs, and provides comprehensive example code. You can use this documentation to write GitHub Pages documentation and README files.
