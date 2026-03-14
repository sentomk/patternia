<div align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)"
            srcset="https://wordpress-1316673449.cos.ap-beijing.myqcloud.com/img/banner-dark.svg">
    <source media="(prefers-color-scheme: light)"
            srcset="https://wordpress-1316673449.cos.ap-beijing.myqcloud.com/img/banner-dark.svg">
    <img alt="Patternia logo"
         src="https://wordpress-1316673449.cos.ap-beijing.myqcloud.com/img/banner-dark.svg"
         width="300"
         style="max-width: 90%; height: auto; margin-top: 10px; transform: translateX(-2px);">
  </picture>
</div>

<br>

<div align="center">

[![C++17+](https://img.shields.io/badge/C%2B%2B-17%2B-blue.svg?style=flat-square&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Build](https://github.com/SentoMK/patternia/actions/workflows/ci.yml/badge.svg?branch=main&style=flat-square)](https://github.com/SentoMK/patternia/actions)
[![License](https://img.shields.io/github/license/SentoMK/patternia?style=flat-square)](LICENSE)
[![Version](https://img.shields.io/github/v/release/SentoMK/patternia?style=flat-square&color=orange)](https://github.com/SentoMK/patternia/releases)
[![Docs](https://img.shields.io/badge/docs-online-blue?style=flat-square)](https://patternia.tech)

</div>

<br>

**Patternia** is a header-only pattern matching library for modern C++.
It keeps matching expression-oriented, explicit, and zero-overhead.

> **Upcoming in next release:** `lit<V>()` will be replaced by `val<V>` and
> `has<Ms...>()` will become `has<Ms...>` (variable templates).
> See [Roadmap](https://patternia.tech/roadmap/) for details.

## Syntax

```cpp
#include <ptn/patternia.hpp>

int classify(int x) {
  using namespace ptn;

  return match(x) | on(
    lit(0) >> 0,
    lit(1) >> 1,
    _ >> -1
  );
}
```

`match(subject)` creates the evaluation context.
`on(...)` provides the ordered case list.
`pattern >> handler` defines one case.
`_` is the required fallback case.

## Highlights

- Literal, structural, and `std::variant` matching in one DSL.
- Explicit binding through `$` and `$(...)`.
- Declarative guards via `PTN_LET`, `PTN_WHERE`, `_0`, `arg<N>`, `rng(...)`, and callables.
- No RTTI, no virtual dispatch, no heap allocation.
- Static literal and variant dispatch lowering for hot paths.

## Quick Examples

### Guarded value match

```cpp
using namespace ptn;

const char *bucket(int x) {
  return match(x) | on(
    $[PTN_LET(value, value < 0)] >> "negative",
    $[PTN_LET(value, value < 10)] >> "small",
    _ >> "large"
  );
}
```

### Structural match

```cpp
using namespace ptn;

struct Point { int x; int y; };

int magnitude2(const Point &p) {
  return match(p) | on(
    $(has<&Point::x, &Point::y>()) >> [](int x, int y) {
      return x * x + y * y;
    },
    _ >> 0
  );
}
```

### Variant match

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

## Installation

Patternia is header-only with no external dependencies.

**vcpkg** (recommended):

```bash
vcpkg install patternia
```

```cmake
find_package(patternia CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE patternia::patternia)
```

**FetchContent**:

```cmake
include(FetchContent)
FetchContent_Declare(patternia
  GIT_REPOSITORY https://github.com/sentomk/patternia.git
  GIT_TAG v0.9.0
)
FetchContent_MakeAvailable(patternia)

target_link_libraries(your_target PRIVATE patternia::patternia)
```

**Direct clone**:

```bash
git clone https://github.com/SentoMK/patternia.git
cd patternia
cmake -S . -B build
cmake --build build
```

See [Installation Guide](https://patternia.tech/guide/installation/) for `find_package`, submodule, and header-copy options.

## Tests

```bash
cmake -S . -B build -DPTN_BUILD_TESTS=ON
cmake --build build --target ptn_tests
ctest --test-dir build --output-on-failure
```

## Performance-Oriented Usage

Cache the case pack for repeated hot paths:

```cpp
using namespace ptn;

int fast_classify(int x) {
  return match(x) | PTN_ON(
    lit<1>() >> 1,
    lit<2>() >> 2,
    _ >> 0
  );
}
```

`PTN_ON(...)` is a convenience wrapper over `static_on(...)`.
It avoids rebuilding the matcher object on every call.

## Documentation

- [Getting Started](https://patternia.tech/guide/getting-started/)
- [API Reference](https://patternia.tech/api/)
- [Design Overview](https://patternia.tech/design-overview/)
- [Tutorials](https://patternia.tech/tutorials/from-control-flow/)
- [Performance Notes](https://patternia.tech/performance/)

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) before sending changes.
This project is governed by [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
