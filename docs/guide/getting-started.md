# Getting Started

## Install

```bash
git clone https://github.com/SentoMK/patternia.git
cd patternia
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Patternia is header-only.
Including `ptn/patternia.hpp` is enough to use the library.

---

## First Match

```cpp
#include <ptn/patternia.hpp>

int classify(int x) {
  using namespace ptn;

  return match(x) | on(
    lit(0) >> 0,
    lit(5) >> 5,
    _ >> -1
  );
}
```

Read this as:

1. Start matching `x`.
2. Try the cases in order.
3. Use `_` for the final fallback.

---

## Side Effects and Returned Values

Patternia matches are expressions.
Handlers can return values or perform side effects.

```cpp
using namespace ptn;

void log_value(int x) {
  match(x) | on(
    lit(1) >> [] { std::cout << "one\n"; },
    lit(2) >> [] { std::cout << "two\n"; },
    _ >> [] { std::cout << "other\n"; }
  );
}
```

```cpp
using namespace ptn;

int score(int x) {
  return match(x) | on(
    lit(1) >> 100,
    lit(2) >> 200,
    _ >> 0
  );
}
```

---

## Binding

Use `$` or `$(...)` when the handler needs data from the match.

```cpp
using namespace ptn;

int identity(int x) {
  return match(x) | on(
    $ >> [](int v) { return v; },
    _ >> 0
  );
}
```

Use `$` to bind the whole subject.
Use `$(...)` to bind under a subpattern.

---

## Guards

Attach a guard with `[]`.

```cpp
using namespace ptn;

const char *bucket(int x) {
  return match(x) | on(
    $[_ > 0 && _ < 10] >> "small",
    _ >> "other"
  );
}
```

For structural guards, give members readable placeholder names with a C++17
structured binding:

```cpp
using namespace ptn;

struct Point {
  int x;
  int y;
};

constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;

bool on_unit_circle(const Point &p) {
  return match(p) | on(
    $(pt)[x * x + y * y == 1] >> true,
    _ >> false
  );
}

bool on_diagonal(const Point &p) {
  return match(p) | on(
    $(pt)[x == y] >> true,
    _ >> false
  );
}
```

`PTN_BIND` is deprecated but still supported. Names declared at namespace
scope as above also work with `PTN_ON`. Inside a function, automatic
structured bindings work with `on(...)`, not with `PTN_ON`'s captureless
lambda. In C++20, make a function-local pattern `static constexpr` and use
`static auto [x, y] = pt;` for its names. In C++17, retain `PTN_BIND` for local
names and make any local pattern used by `PTN_ON` static too. See the
[API reference](../api.md#scope-and-cached-matchers) for the scope rules.

---

## Structural Matching

Use `has<>` to describe object structure and `$(...)` to bind selected members.

```cpp
using namespace ptn;

struct Point {
  int x;
  int y;
};

int sum(const Point &p) {
  return match(p) | on(
    $(has<&Point::x, &Point::y>) >> [](int x, int y) {
      return x + y;
    },
    _ >> 0
  );
}
```

---

## Variant Matching

Use `is<T>` for type-based dispatch and `alt<I>` for index-based dispatch.

```cpp
using namespace ptn;

using Value = std::variant<int, std::string>;

std::string describe(const Value &v) {
  return match(v) | on(
    is<int> >> "int",
    $(is<std::string>) >> [](const std::string &s) {
      return "str:" + s;
    },
    _ >> [] { return std::string("other"); }
  );
}
```

---

## Cached Hot Paths

If the same case pack is reused on a hot path, cache it:

```cpp
using namespace ptn;

int classify_fast(int x) {
  return match(x) | PTN_ON(
    val<1> >> 1,
    val<2> >> 2,
    _ >> 0
  );
}
```

Or:

```cpp
using namespace ptn;

int classify_fast(int x) {
  return match(x) | static_on([] {
    return on(
      val<1> >> 1,
      val<2> >> 2,
      _ >> 0
    );
  });
}
```

---

## Tests

```bash
cmake -S . -B build -DPTN_BUILD_TESTS=ON
cmake --build build --target ptn_tests
ctest --test-dir build --output-on-failure
```

---

## Next

- Read the [API reference](../api.md)
- Read the [design overview](../design-overview.md)
- Continue with the tutorials under `docs/tutorials/`
