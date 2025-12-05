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
│   ├── value/                        // Value patterns
│   └── type/                         // Type patterns
└── meta/                             // Meta programming tools
    ├── base/                         // Base traits
    ├── dsa/                          // Data structures and algorithms
    └── query/                        // Query utilities
```

### Main Namespace Aliases

For user convenience, Patternia provides the following namespace aliases:

```cpp
namespace ptn {
  namespace type = pat::type;    // Type pattern alias
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

### Relational Patterns

#### `ptn::lt(value)` - Less than

```cpp
template <typename V>
constexpr auto lt(V &&v);
```

**Example:**
```cpp
match(5).when(lt(10) >> "less-than-10");
```

#### `ptn::le(value)` - Less than or equal

```cpp
template <typename V>
constexpr auto le(V &&v);
```

**Example:**
```cpp
match(10).when(le(10) >> "less-or-equal-10");
```

#### `ptn::gt(value)` - Greater than

```cpp
template <typename V>
constexpr auto gt(V &&v);
```

**Example:**
```cpp
match(15).when(gt(10) >> "greater-than-10");
```

#### `ptn::ge(value)` - Greater than or equal

```cpp
template <typename V>
constexpr auto ge(V &&v);
```

**Example:**
```cpp
match(10).when(ge(10) >> "greater-or-equal-10");
```

#### `ptn::eq(value)` - Equal

```cpp
template <typename V>
constexpr auto eq(V &&v);
```

**Example:**
```cpp
match(42).when(eq(42) >> "equal-42");
```

#### `ptn::ne(value)` - Not equal

```cpp
template <typename V>
constexpr auto ne(V &&v);
```

**Example:**
```cpp
match(42).when(ne(0) >> "not-zero");
```

#### `ptn::between(lo, hi, closed)` - Range matching

```cpp
template <typename L, typename R>
constexpr auto between(L &&lo, R &&hi, bool closed = true);
```

**Parameters:**
- `lo` - Lower bound
- `hi` - Upper bound
- `closed` - Whether it's a closed interval (default true)

**Examples:**
```cpp
match(5).when(between(1, 10) >> "in-range");      // [1, 10]
match(5).when(between(1, 10, false) >> "in-open-range"); // (1, 10)
```

### Predicate Patterns

#### `ptn::pred(predicate)`

Wrap any predicate function as a pattern.

```cpp
template <typename F>
constexpr auto pred(F &&f);
```

**Parameters:** `predicate` - Callable object that accepts one parameter and returns a value convertible to bool

**Examples:**
```cpp
auto is_even = pred([](int x) { return x % 2 == 0; });
auto is_positive = pred([](int x) { return x > 0; });

match(4).when(is_even >> "even-number");
match(-5).when(is_positive >> "positive");
```

---

## Type Pattern API

!!! note
    **`ptn::pat::type::is` = `ptn::type::is`** 

### `ptn::pat::type::is<T>`

Exact type matching pattern.

```cpp
template <typename T>
inline constexpr detail::is_pattern<T> is{};
```

**Purpose:** Match types identical to the given type

**Bound Value:** The matched value itself

**Examples:**
```cpp
match(42).when(pat::type::is<int> >> "integer");
match(3.14).when(pat::type::is<double> >> "floating-point");

// Alias in ptn::type namespace
match(42).when(type::is<int> >> "integer");
```

### `ptn::pat::type::in<Ts...>`

Type set matching pattern.

```cpp
template <typename... Ts>
inline constexpr detail::in_pattern<Ts...> in{};
```

**Purpose:** Match types belonging to any type in the given type set

**Parameters:** `Ts...` - Type list

**Bound Value:** The matched value itself

**Examples:**
```cpp
match(42).when(pat::type::in<int, long, short> >> "integral");
match(3.14f).when(pat::type::in<float, double> >> "floating");

// Using alias
match("hello").when(type::in<std::string, std::string_view> >> "string-like");
```

### `ptn::pat::type::not_in<Ts...>`

Type exclusion matching pattern.

```cpp
template <typename... Ts>
inline constexpr detail::not_in_pattern<Ts...> not_in{};
```

**Purpose:** Match types not belonging to any type in the given type set

**Parameters:** `Ts...` - Type list to exclude

**Bound Value:** The matched value itself

**Examples:**
```cpp
match(42).when(pat::type::not_in<float, double> >> "not-floating");
match("hello").when(pat::type::not_in<int, double> >> "not-number");

// Using alias
match(3.14f).when(type::not_in<int, long> >> "not-integral");
```

### `ptn::pat::type::from<Tpl>`

Template specialization matching pattern.

```cpp
template <template <typename...> typename Tpl>
inline constexpr detail::from_pattern<Tpl> from{};
```

**Purpose:** Match specializations of a specific template

**Parameters:** `Tpl` - Template name

**Bound Value:** The matched value itself

**Examples:**
```cpp
#include <vector>
#include <optional>
#include <map>

std::vector<int> vec{1, 2, 3};
std::optional<std::string> opt = "hello";
std::map<int, std::string> map{{1, "one"}};

match(vec).when(pat::type::from<std::vector> >> "vector");
match(opt).when(pat::type::from<std::optional> >> "optional");
match(map).when(pat::type::from<std::map> >> "map");

// Using alias
match(vec).when(type::from<std::vector> >> "vector");
```

---

## DSL Operators API

### Pattern Combination Operators

#### `pattern1 && pattern2` - Logical AND

Create a combined pattern that satisfies both patterns simultaneously.

```cpp
template <typename L, typename R>
constexpr auto operator&&(L &&l, R &&r);
```

**Examples:**
```cpp
auto positive_even = pred([](int x) { return x > 0; }) && 
                     pred([](int x) { return x % 2 == 0; });

match(4).when(positive_even >> "positive-even");
match(-2).when(positive_even >> "not-positive-even");

// Mix type and value patterns
match(42).when(pat::type::is<int> && lit(42) >> "exact-42-int");
```

#### `pattern1 || pattern2` - Logical OR

Create a combined pattern that satisfies either pattern.

```cpp
template <typename L, typename R>
constexpr auto operator||(L &&l, R &&r);
```

**Examples:**
```cpp
auto zero_or_empty = lit(0) || lit(std::optional<int>{});

match(0).when(zero_or_empty >> "zero-or-empty");
match(std::optional<int>{}).when(zero_or_empty >> "zero-or-empty");

// Type combination
auto numeric = pat::type::is<int> || pat::type::is<double>;
match(42).when(numeric >> "numeric");
match(3.14).when(numeric >> "numeric");
```

#### `!pattern` - Logical NOT

Create a pattern that does not satisfy the given pattern.

```cpp
template <typename P>
constexpr auto operator!(P &&p);
```

**Examples:**
```cpp
auto not_zero = !lit(0);
auto not_string = !pat::type::is<std::string>;

match(5).when(not_zero >> "not-zero");
match(42).when(not_string >> "not-string");
```

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
match(42).when(lit(42) >> [](int x) { 
    return x * 2; 
});  // Returns 84

// Handler with binding (type pattern)
match("hello").when(pat::type::is<std::string> >> [](const std::string& s) {
    return s.length();
});  // Returns 5
```

---

## Metaprogramming API

### Basic Type Traits

#### `ptn::meta::remove_cvref_t<T>`

Type alias that removes const, volatile, and reference qualifiers.

```cpp
template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;
```

**Example:**
```cpp
static_assert(std::is_same_v<
    ptn::meta::remove_cvref_t<const int&>, 
    int
>);
```

#### `ptn::meta::is_spec_of_v<Tpl, T>`

Check if a type is a specialization of a specific template.

```cpp
template <template <typename...> typename Template, typename T>
inline constexpr bool is_spec_of_v = is_spec_of<Template, T>::value;
```

**Example:**
```cpp
static_assert(ptn::meta::is_spec_of_v<std::vector, std::vector<int>>);
static_assert(!ptn::meta::is_spec_of_v<std::vector, int>);
```

#### `ptn::meta::is_tmpl_v<T>`

Check if a type is a specialization of some template.

```cpp
template <typename T>
inline constexpr bool is_tmpl_v = is_tmpl<T>::value;
```

**Example:**
```cpp
static_assert(ptn::meta::is_tmpl_v<std::vector<int>>);
static_assert(!ptn::meta::is_tmpl_v<int>);
```

### Type List Operations

#### `ptn::meta::type_list<Ts...>`

Compile-time type container.

```cpp
template <typename... Ts>
struct type_list {
    static constexpr std::size_t size = sizeof...(Ts);
};
```

#### List Operation Functions

##### `contact(list1, list2)` - Concatenate two type lists

```cpp
template <typename... A, typename... B>
constexpr auto contact(type_list<A...>, type_list<B...>) -> type_list<A..., B...>;
```

##### `append<T>(list)` - Add type to end of list

```cpp
template <typename T, typename... Ts>
constexpr auto append(type_list<Ts...>) -> type_list<Ts..., T>;
```

##### `prepend<T>(list)` - Add type to beginning of list

```cpp
template <typename T, typename... Ts>
constexpr auto prepend(type_list<Ts...>) -> type_list<T, Ts...>;
```

##### `nth_type_t<N, list>` - Get the Nth type

```cpp
template <std::size_t N, typename TL>
using nth_type_t = typename nth_type<N, TL>::type;
```

#### Type List Algorithms

##### `ptn::meta::all<Bs...>` - Logical AND fold

```cpp
template <bool... Bs>
struct all : std::bool_constant<(Bs && ...)> {};
```

##### `ptn::meta::any<Bs...>` - Logical OR fold

```cpp
template <bool... Bs>
struct any : std::bool_constant<(Bs || ...)> {};
```

##### `ptn::meta::none<Bs...>` - Logical NOT fold

```cpp
template <bool... Bs>
struct none : std::bool_constant<!any<Bs...>::value> {};
```

##### `ptn::meta::map_t<F, list>` - Apply metafunction to each type

```cpp
template <template <typename> typename F, typename TL>
using map_t = typename map<F, TL>::type;
```

##### `ptn::meta::filter_t<Pred, list>` - Filter type list

```cpp
template <template <typename> typename Pred, typename TL>
using filter_t = typename filter<Pred, TL>::type;
```

### Query Operations

#### `ptn::meta::contains_v<T, list>` - Check if type is in list

```cpp
template <typename T, typename TL>
inline constexpr bool contains_v = contains<T, TL>::value;
```

#### `ptn::meta::index_of_v<T, list>` - Get index of type in list

```cpp
template <typename T, typename TL>
inline constexpr int index_of_v = index_of<T, TL>::value;
```

**Returns:** Index of the type in the list, returns -1 if not found

### Template Information Queries

#### `ptn::meta::template_info<T>`

Extract template specialization information.

```cpp
template <typename T>
struct template_info {
    static constexpr bool is_specialization = false;
    using args = type_list<>;
    // Non-template types don't have rebind
};
```

**Specialization version:**
```cpp
template <template <typename...> typename Template, typename... Args>
struct template_info<Template<Args...>> {
    static constexpr bool is_specialization = true;
    using args = type_list<Args...>;
    
    template <typename... Us>
    using rebind = Template<Us...>;
};
```

**Example:**
```cpp
using info = ptn::meta::template_info<std::vector<int>>;
static_assert(info.is_specialization);
using args = info.args;  // type_list<int>
using new_vec = info::template rebind<double>;  // std::vector<double>
```

---

## Complete Examples

### Basic Usage Examples

```cpp
#include "ptn/patternia.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <optional>

using namespace ptn;
using namespace ptn::pat::value;
using namespace ptn::pat::type;

void basic_examples() {
    // 1. Simple value matching
    int x = 42;
    auto result1 = match(x)
        .when(lit(42) >> "answer to everything")
        .when(lit(0) >> "zero")
        .otherwise("other number");
    
    // 2. Relational patterns
    double score = 85.5;
    auto result2 = match(score)
        .when(ge(90) >> 'A')
        .when(ge(80) >> 'B')
        .when(ge(70) >> 'C')
        .when(ge(60) >> 'D')
        .otherwise('F');
    
    // 3. Type patterns
    std::variant<int, std::string, double> var = std::string("hello");
    auto result3 = match(var)
        .when(type::is<int> >> [](int i) { return i * 2; })
        .when(type::is<std::string> >> [](const std::string& s) { return s.length(); })
        .when(type::is<double> >> [](double d) { return static_cast<int>(d); });
    
    std::cout << "Result 1: " << result1 << '\n';
    std::cout << "Result 2: " << result2 << '\n';
    std::cout << "Result 3: " << result3 << '\n';
}
```

### Complex Pattern Combination Examples

```cpp
void complex_patterns() {
    // 1. Combined pattern: positive even numbers
    auto positive_even = pred([](int x) { return x > 0; }) && 
                         pred([](int x) { return x % 2 == 0; });
    
    // 2. Range pattern: numbers between 10 and 20
    auto in_range = between(10, 20);
    
    // 3. Type combination: numeric types
    auto numeric = type::is<int> || type::is<double> || type::is<float>;
    
    // 4. Complex matching chain
    auto result = match(15)
        .when(positive_even >> "positive even")
        .when(in_range && type::is<int> >> "integer in range")
        .when(numeric >> "some number")
        .otherwise("other");
    
    // 5. Container type matching
    std::vector<int> vec{1, 2, 3};
    std::optional<std::string> opt = "optional value";
    std::map<int, std::string> map{{1, "one"}};
    
    auto container_type = [&](auto&& container) {
        return match(container)
            .when(type::from<std::vector> >> "vector")
            .when(type::from<std::optional> >> "optional")
            .when(type::from<std::map> >> "map")
            .otherwise("unknown container");
    };
    
    std::cout << "Complex result: " << result << '\n';
    std::cout << "Vec type: " << container_type(vec) << '\n';
    std::cout << "Opt type: " << container_type(opt) << '\n';
    std::cout << "Map type: " << container_type(map) << '\n';
}
```

### String Processing Examples

```cpp
void string_examples() {
    std::string input = "Hello";
    
    auto result = match(input)
        .when(lit_ci("hello") >> "greeting (case insensitive)")
        .when(lit("world") >> "world")
        .when(pred([](const std::string& s) { return s.length() > 5; }) >> "long string")
        .otherwise("other string");
    
    std::cout << "String result: " << result << '\n';
}
```

### Enum and User Type Examples

```cpp
enum class Status {
    Active,
    Inactive,
    Pending,
    Error
};

struct User {
    std::string name;
    Status status;
    int age;
};

void custom_types_example() {
    Status status = Status::Active;
    auto status_result = match(status)
        .when(lit(Status::Active) >> "user is active")
        .when(lit(Status::Inactive) >> "user is inactive")
        .when(lit(Status::Pending) >> "user is pending")
        .when(lit(Status::Error) >> "error state");
    
    User user{"Alice", Status::Active, 25};
    auto user_result = match(user)
        .when(pred([](const User& u) { return u.status == Status::Active && u.age >= 18; }) 
              >> "active adult user")
        .when(pred([](const User& u) { return u.status == Status::Pending; }) 
              >> "pending user")
        .otherwise("other user");
    
    std::cout << "Status: " << status_result << '\n';
    std::cout << "User: " << user_result << '\n';
}
```

### Performance Analysis Examples

```cpp
void performance_analysis() {
    // Compile-time optimization: all matches determined at compile time
    constexpr auto get_description = [](int value) constexpr {
        return match(value)
            .when(lit(0) >> "zero")
            .when(lit(1) >> "one")
            .when(lit(2) >> "two")
            .when(pred([](int x) { return x > 2; }) >> "greater than two")
            .otherwise("negative");
    };
    
    static_assert(get_description(0) == "zero");
    static_assert(get_description(3) == "greater than two");
    
    // Runtime usage
    for (int i = -1; i <= 3; ++i) {
        std::cout << i << ": " << get_description(i) << '\n';
    }
}
```

### Metaprogramming Usage Examples

```cpp
void metaprogramming_examples() {
    // Using type lists
    using numeric_types = ptn::meta::type_list<int, float, double>;
    
    // Check if type is in list
    static_assert(ptn::meta::contains_v<int, numeric_types>);
    static_assert(!ptn::meta::contains_v<std::string, numeric_types>);
    
    // Get type index
    static_assert(ptn::meta::index_of_v<double, numeric_types> == 2);
    static_assert(ptn::meta::index_of_v<char, numeric_types> == -1);
    
    // Template information query
    using vec_int_info = ptn::meta::template_info<std::vector<int>>;
    static_assert(vec_int_info::is_specialization);
    using vec_args = vec_int_info::args;  // type_list<int, std::allocator<int>>
    
    // Rebind template parameters
    using vec_double = vec_int_info::template rebind<double>;  // std::vector<double>
    
    // Compile-time algorithms
    static_assert(ptn::meta::all<true, true, false>::value == false);
    static_assert(ptn::meta::any<true, false, false>::value == true);
    static_assert(ptn::meta::none<false, false>::value == true);
}
```

### Complete Comprehensive Example

```cpp
#include "ptn/patternia.hpp"
#include <iostream>
#include <variant>
#include <vector>
#include <optional>
#include <map>

using namespace ptn;
using namespace ptn::pat::value;
using namespace ptn::pat::type;

class PatterniaDemo {
public:
    void runAllExamples() {
        std::cout << "=== Patternia API Examples ===\n\n";
        
        basic_examples();
        std::cout << '\n';
        
        complex_patterns();
        std::cout << '\n';
        
        string_examples();
        std::cout << '\n';
        
        custom_types_example();
        std::cout << '\n';
        
        performance_analysis();
        std::cout << '\n';
        
        std::cout << "=== All Examples Complete ===\n";
    }
};

int main() {
    PatterniaDemo demo;
    demo.runAllExamples();
    return 0;
}
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
