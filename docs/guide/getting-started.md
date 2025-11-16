# Getting Started

Patternia is a header-only library.
Once installed or fetched into your project, you can start using pattern matching immediately.

Below is the minimal working example demonstrating the core DSL:

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
      .when(lt(0) >> "neg")
      .when(gt(0) >> "pos")
      .otherwise("other");
```

---


## Basic Example

```cpp
#include <ptn/patternia.hpp>
#include <iostream>
using namespace ptn;

int main() {
  int x = 42;

  auto result =
    match(x)
      .when(lit(0)       >> "zero")
      .when(lit(42)      >> "answer to everything")
      .when(pred([](int v){ return v % 2 == 0; }) >> "even")
      .otherwise("other");

  std::cout << result << "\n";
}
```

Output:

```
answer to everything
```

---

## Type-based Matching

Patternia provides a type layer for matching based on static type.

```cpp
std::variant<int, float, std::string> v = 3.14f;

auto out = match(v)
  .when(type::is<int>    >> "int")
  .when(type::is<float>  >> "float")
  .when(type::is<std::string> >> "string")
  .otherwise("unknown");
```

---

## Relational Patterns

Patternia’s relational DSL (`>`, `<`, `>=`, `<=`) works on any comparable value.

```cpp
int score = 87;

auto grade = match(score)
  .when(lt(60)             >> "F") // less than
  .when(ge(90)             >> "A") // greater equal
  .when(ge(80)             >> "B")
  .when(ge(70)             >> "C")
  .otherwise("D");
```

---

## Predicates

Arbitrary logic can be wrapped into concise predicate patterns:

```cpp
auto res = match(x)
  .when(pred([](int v){ return v % 2 == 1; }) >> "odd")
  .otherwise("even");
```

---

## Case-insensitive matching

```cpp
std::string s = "Ok";

auto out = match(s)
  .when(lit_ci("OK") >> "accepted")
  .otherwise("rejected");
```

---

## No macros. No reflection. No magic

Patternia compiles down to normal C++ control flow using inlined pattern objects.
There is no runtime overhead beyond executing the selected handler.