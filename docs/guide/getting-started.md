# Getting Started

Patternia is a header-only library.
Once installed or fetched into your project, you can start using pattern matching immediately.

Below is minimal working example demonstrating the core DSL:

---

## Optional: Recommended Code Style

To keep your chained `.when()` and `.otherwise()` expressions visually aligned and easy to scan, you can add a minimal `.clang-format` to your project root:

```yaml
# patternia .clang-format (minimal)
BasedOnStyle: LLVM
IndentWidth: 2            # or 4
ContinuationIndentWidth: 4 # or 6
ColumnLimit: 0
BinPackArguments: false
BinPackParameters: false
BreakBeforeBinaryOperators: None
```

With this style, multi-line match expressions remain clean and consistent:

```cpp
auto out =
    match(5)
      .when(lit(0) >> "zero")
      .when(lit(5) >> "five")
      .otherwise("other");
```

## 1. Installation

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
#include "ptn/patternia.hpp"
```

**Notes:**

* `GIT_TAG main` tracks the latest development version
* Patternia is header-only — no targets need to be linked
* Requires **C++17 or later**



### By vcpkg

vcpkg support is **coming soon**.


### Other Installation Methods

For additional installation options and detailed setup instructions (including manual integration and future package manager support), see:

👉 [Patternia Installation](https://sentomk.github.io/patternia/guide/installation/)


## 2. Matching Values

### Basic DSL Syntax

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

### Pattern Fallback with `__` and `.end()`

For exhaustive data types where you want to use pattern fallback:

```cpp
#include <iostream>
#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int x = 2;

  match(x)
    .when(lit(1) >> [] { std::cout << "one\n"; })
    .when(lit(2) >> [] { std::cout << "two\n"; })
    .when(__ >> [] { std::cout << "other values\n"; })  // pattern fallback
    .end();  // required for __ to work
}
```

### Tuple-Based Syntax (Simple Cases)

For simple matching scenarios, Patternia provides a concise tuple-based syntax:

```cpp
#include <iostream>
#include <ptn/patternia.hpp>

using namespace ptn;

int main() {
  int x = 2;

  match(x, cases(
    lit(1) >> [] { std::cout << "one\n"; },
    lit(2) >> [] { std::cout << "two\n"; },
    __    >> [] { std::cout << "other\n"; }  // pattern fallback
  )).end();
}
```

**Key points:**

* `match(subject, cases(...))` provides a compact syntax for simple cases
* **No guards/predicates**: This syntax cannot be used with guard expressions or predicates
* Cases are tested **top-to-bottom (first-match semantics)**
* Supports both pattern fallback (`__`) and match fallback patterns
* **Requires `.end()`** to trigger evaluation when using `__` pattern
* Ideal for straightforward value matching without complex conditions

**Limitations:**
- Cannot use guard expressions `[]` with tuple syntax
- Cannot use predicate-based guards
- Best suited for simple literal and wildcard matching
- **Must use `.end()` when using `__` pattern to trigger evaluation**

For complex matching with guards, use the standard DSL syntax shown above.


## 3. Fallback Mechanisms and Match Completion

Patternia supports **value-returning matches**, similar to Rust or Scala.

```cpp
int classify(int x) {
  return match(x)
    .when(lit(0) >> 0)
    .when(lit(1) >> 1)
    .otherwise(-1);  // returns a value
}
```

Both `.otherwise()` and `.end()` can be used for either value-returning or side-effect scenarios. The key distinction is:

* **`.otherwise()`**: Match fallback that works in all scenarios
* **`.end()`**: Required when using `__` pattern fallback

**Using `.otherwise()`:**
```cpp
auto result = match(value)
  .when(lit(1) >> 100)
  .when(lit(2) >> 200)
  .otherwise(0);  // match fallback
```

**Using `.end()` with `__`:**
```cpp
match(value)
  .when(lit(1) >> [] { std::cout << "case 1\n"; })
  .when(lit(2) >> [] { std::cout << "case 2\n"; })
  .when(__ >> [] { std::cout << "fallback\n"; })  // pattern fallback
  .end();  // required for __
```

**Key Point:** `.end()` is specifically required when using the `__` pattern, while `.otherwise()` can be used independently of `__`.


## 4. Binding Values

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


## 5. Guards (Conditional Matching)

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


## 6. Structural Matching

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


### Variant Type Matching (std::variant)

Patternia can match `std::variant` alternatives by type using `type::is<T>()`.
If you want to bind the alternative value, use `as<T>()`, which is explicit
binding sugar for `type::is<T>(bind())`.

```cpp
using V = std::variant<int, std::string, Point>;

match(v)
  .when(type::is<int>() >> [] { /* type-only */ })
  .when(as<std::string>() >> [](const std::string &s) { /* bound */ })
  .when(type::is<Point>(bind(has<&Point::x, &Point::y>())) >>
        [](int x, int y) { /* structural bind */ })
  .otherwise([] {});
```


## 7. Pattern Fallback with `__` and `.end()`

Use `__` as a pattern fallback that **must be paired with `.end()`** to trigger match inference:

```cpp
// For exhaustive data types (integers, enums, etc.)
match(x)
  .when(lit(0) >> [] { std::cout << "zero\n"; })
  .when(lit(1) >> [] { std::cout << "one\n"; })
  .when(__ >> [] { std::cout << "other values\n"; })  // pattern fallback
  .end();  // required for __ to work
```

**Important:** `__` (pattern fallback) **must** be used with `.end()` to enable match inference. Without `.end()`, the `__` case will not trigger.

## 8. Match Fallback with `.otherwise()`

Use `.otherwise()` as a match fallback for scenarios where exhaustive matching is not possible or practical:

```cpp
// For non-exhaustive or complex data types
match(data)
  .when(valid_pattern >> handler)
  .otherwise([] { std::cout << "no match found\n"; });  // match fallback
```

**Key Rules:**

* **`__` + `.end()`**: Use for **exhaustive data types** (integers, enums, etc.) where all possible cases are known
* **`.otherwise()`**: Use when matching is **not guaranteed to be exhaustive** (complex structures, variants, etc.)
* **Cannot mix**: `__`/`.end()` and `.otherwise()` **cannot** be used in the same match expression

## 9. When to Use Pattern Fallback vs Match Fallback

Choose the right fallback strategy based on your data type:

```cpp
// ✅ For exhaustive types - use __ + end()
enum class Color { Red, Green, Blue };
match(color)
  .when(lit(Color::Red) >> [] { /* ... */ })
  .when(lit(Color::Green) >> [] { /* ... */ })
  .when(__ >> [] { /* handles Color::Blue */ })
  .end();

// ✅ For non-exhaustive types - use otherwise()
match(variant_value)
  .when(some_pattern >> handler)
  .otherwise(fallback_handler);

// ❌ WRONG - cannot mix both
match(value)
  .when(pattern >> handler)
  .when(__ >> [] { /* ... */ })
  .otherwise([] { /* ... */ });  // Compile error!
```


## 10. When to Use Patternia

Patternia is particularly effective when:

* branching depends on **data shape**, not just values
* conditions involve **relationships between multiple fields**
* `if` / `switch` logic becomes deeply nested
* you want expression-oriented control flow

For a deeper dive, see the **API Reference** and **Design Guide**.
