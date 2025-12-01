# Patternia Pattern Matching Library Architecture Analysis

## Part 1: Namespace Structure

Patternia library adopts a clear hierarchical namespace structure, divided into the following layers:

### Root Namespace `ptn`
The root namespace of Patternia library, where all public APIs are located.

### Core Sub-namespaces

#### `ptn::core`
Core engine and DSL-related functionality
- `ptn::core::common` - Common utilities and diagnostics
- `ptn::core::dsl` - Domain-specific language operators
- `ptn::core::engine` - Core matching engine implementation

#### `ptn::pat`
Pattern definitions and implementations
- `ptn::pat::base` - Base pattern abstractions and traits
- `ptn::pat::value` - Value patterns (literal, relational, predicate)
- `ptn::pat::type` - Type patterns

#### `ptn::meta`
Metaprogramming utilities
- `ptn::meta::base` - Basic type traits
- `ptn::meta::dsa` - Data structures and algorithms
- `ptn::meta::query` - Type query utilities

### Implementation Detail Namespaces
- `ptn::core::engine::detail` - Engine implementation details
- `ptn::core::dsl::detail` - DSL implementation details
- `ptn::pat::value::detail` - Value pattern implementation details
- `ptn::pat::type::detail` - Type pattern implementation details

## Part 2: Layer Division

Patternia library adopts a classic layered architecture design, divided into the following layers from bottom to top:

### Meta Layer
**Location**: `ptn/meta/`
**Responsibility**: Provides compile-time type operations and metaprogramming infrastructure
- `base/traits.hpp` - Basic type traits (remove_cvref_t, is_spec_of, etc.)
- `dsa/` - Data structures and algorithms (type_list, algorithms)
- `query/` - Type query utilities (index, template_info)

### Core Layer
**Location**: `ptn/core/`
**Responsibility**: Provides core pattern matching engine and DSL support

#### Common Sublayer
- `common/` - Common utilities, diagnostics, optimization, evaluation

#### DSL Sublayer  
- `dsl/` - Domain-specific language operators (>>, &&, ||, !)

#### Engine Sublayer
- `engine/` - Matching engine implementation (builder, match)

### Pattern Layer
**Location**: `ptn/pattern/`
**Responsibility**: Defines various types of patterns and pattern factories

#### Base Sublayer
- `base/` - Pattern base classes, traits, binding contracts

#### Value Pattern Sublayer
- `value/` - Value patterns (literal, relational comparison, predicate)

#### Type Pattern Sublayer  
- `type/` - Type patterns (is, in, not_in, from)

### Configuration Layer
**Location**: `ptn/config.hpp`
**Responsibility**: Compile-time configuration switches and feature control

### Integration Layer
**Location**: `ptn/patternia.hpp`
**Responsibility**: Unified public entry point, exports all necessary headers

## Part 3: API Summary and Semantics

### Core Matching APIs

#### `ptn::match(T&& value)`
**Semantics**: Entry point for pattern matching, creates a match builder
**Parameters**: Value to be matched
**Returns**: `match_builder` object supporting chained calls

#### `ptn::match<U>(T&& value)`
**Semantics**: Pattern matching entry point with explicit type specification
**Parameters**: Target type U and value to be converted
**Returns**: Typed `match_builder` object

### Pattern Builder APIs

#### `when(case_expr)`
**Semantics**: Add a matching condition and corresponding handler function
**Parameters**: Case expression created using `>>` operator
**Returns**: New match builder (supports chained calls)

#### `otherwise(handler)`
**Semantics**: Set default handler function, executed when all patterns fail to match
**Parameters**: Handler function or return value
**Returns**: Matching result

### DSL Operator APIs

#### `pattern >> handler`
**Semantics**: Associate pattern with handler function
**Special**: If handler is a value type, automatically converted to a function returning that value

#### `pattern1 && pattern2`
**Semantics**: Logical AND composition, both patterns must match

#### `pattern1 || pattern2`
**Semantics**: Logical OR composition, either pattern matching is sufficient

#### `!pattern`
**Semantics**: Logical NOT, succeeds when pattern does not match

### Value Pattern APIs

#### Literal Patterns
- `lit(value)` - Exact literal value matching
- `lit_ci(value)` - Case-insensitive string matching

#### Relational Patterns
- `lt(value)` - Less than (x < value)
- `le(value)` - Less than or equal (x <= value)
- `gt(value)` - Greater than (x > value)
- `ge(value)` - Greater than or equal (x >= value)
- `eq(value)` - Equal (x == value)
- `ne(value)` - Not equal (x != value)
- `between(lo, hi, closed=true)` - Interval matching

#### Predicate Patterns
- `pred(predicate)` - Use arbitrary predicate function

### Type Pattern APIs

#### `ptn::pat::type::is<T>`
**Semantics**: Exact type matching, binds value to handler when match succeeds
**Binding**: Binds matched value as `std::tuple<Subject>`

#### `ptn::pat::type::in<Types...>`
**Semantics**: Type set matching, checks if type is in given type list

#### `ptn::pat::type::not_in<Types...>`
**Semantics**: Type exclusion matching, checks if type is not in given type list

#### `ptn::pat::type::from<T>`
**Semantics**: Type conversion matching, checks if convertible from target type

### Metaprogramming APIs

#### Type Traits
- `ptn::meta::remove_cvref_t<T>` - Remove const/volatile and reference
- `ptn::meta::is_spec_of_v<Template, T>` - Check if template specialization
- `ptn::meta::is_tmpl_v<T>` - Check if template instance

#### Type List Operations
- `ptn::meta::dsa::type_list` - Type list container
- `ptn::meta::dsa::algorithms` - Type list algorithms

### Configuration Macros
- `PTN_ENABLE_VALUE_PATTERN` - Enable value patterns
- `PTN_ENABLE_RELATIONAL_PATTERN` - Enable relational patterns
- `PTN_ENABLE_TYPE_PATTERN` - Enable type patterns
- `PTN_ENABLE_PREDICATE_PATTERN` - Enable predicate patterns
- `PTN_USE_CONCEPTS` - Enable C++20 concepts support

### Binding Semantics
- **Literal patterns**: Bind no values (`std::tuple<>`)
- **Relational patterns**: Bind no values (`std::tuple<>`)
- **Predicate patterns**: Bind no values (`std::tuple<>`)
- **Type patterns**: Bind matched value (`std::tuple<Subject>`)

### Error Handling
The library uses `static_assert` for compile-time validation of:
- Pattern type validity
- Handler function signature compatibility
- Type constructibility
- Operator support

This design ensures type safety and high performance while providing an intuitive and easy-to-use DSL syntax.
