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
    $[PTN_LET(value, value > 0 && value < 10)] >> "small",
    _ >> "other"
  );
}
```

For multiple bound values, use `arg<N>` or `PTN_WHERE((...), expr)`:

```cpp
using namespace ptn;

struct Point {
  int x;
  int y;
};

bool on_unit_circle(const Point &p) {
  return match(p) | on(
    $(has<&Point::x, &Point::y>)[arg<0> * arg<0> + arg<1> * arg<1> == 1]
        >> true,
    _ >> false
  );
}
```

```cpp
using namespace ptn;

bool on_diagonal(const Point &p) {
  return match(p) | on(
    $(has<&Point::x, &Point::y>)[PTN_WHERE((x, y), x == y)] >> true,
    _ >> false
  );
}
```

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
    lit<1>() >> 1,
    lit<2>() >> 2,
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
      lit<1>() >> 1,
      lit<2>() >> 2,
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
