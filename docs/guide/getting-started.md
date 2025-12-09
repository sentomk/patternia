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
      .otherwise("other");

  std::cout << result << "\n";
}
```

Output:

```
answer to everything
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
