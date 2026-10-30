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
[![Coverage](https://img.shields.io/codecov/c/github/SentoMK/patternia?style=flat-square&branch=main)](https://codecov.io/gh/SentoMK/patternia)
[![Docs](https://img.shields.io/badge/docs-online-blue?style=flat-square)](https://patternia.tech)

</div>

<br>

**Patternia** is a header-only pattern matching library for C++17 and later.
Match literals, `std::variant` alternatives, and struct members, bind the
values you need, and guard them, all in one expression:

```cpp
#include <ptn/patternia.hpp>
#include <string>
#include <variant>
using namespace ptn;

using Value = std::variant<int, std::string>;

std::string describe(const Value &v) {
  return match(v) | on(
    is<int>(lit(0))             >> "zero",
    $(is<int>)[_ < 0]           >> [](int n) { return "negative " + std::to_string(n); },
    $(is<int>)                  >> [](int n) { return "positive " + std::to_string(n); },
    $(is<std::string>)[_ != ""] >> [](const std::string &s) { return "text: " + s; },
    _                           >> "empty"
  );
}
```

`$(...)` binds the matched value and passes it to the handler, `[...]` is a
guard that must also hold, and the first matching case wins.

## Highlights

- **One DSL for many shapes**: runtime and compile-time literals, ASCII
  case-insensitive strings, struct members, and `std::variant`
  alternatives (by type or by index).
- **Expression-oriented**: `match(subject) | on(...)` returns a value;
  cases are ordered and first-match-wins, with a required `_` fallback.
- **Explicit binding**: nothing binds implicitly. `$` binds the subject,
  `$(pattern)` binds what a subpattern exposes, and handler parameters
  follow binding order.
- **Declarative guards**: `$[_ > 0 && _ < 10]`, `rng(lo, hi)`, named
  struct-member placeholders via C++17 structured bindings, or any
  callable.
- **Pattern algebra**: `any`, `all`, `neg`, with operator sugar
  `(a || b)`, `(a && b)`, and `!p`.
- **Compile-time checks**: a missing `_` fallback, unreachable cases,
  out-of-range `has<...>` indices, and invalid member names are
  diagnosed at compile time.
- **Zero-overhead goals**: no RTTI, no virtual dispatch, no heap
  allocation; static dispatch lowering for compile-time literals and
  variant cases, and `PTN_ON(...)` to cache case packs on hot paths.
- **No dependencies**: just headers. Packaged for vcpkg and exported as
  a CMake target.

## Quick Tour

All examples assume `#include <ptn/patternia.hpp>`, the standard headers
they use, and `using namespace ptn;`.

### Literals and value guards

```cpp
const char *bucket(int x) {
  return match(x) | on(
    lit(0)    >> "zero",
    $[_ < 0]  >> "negative",   // `_` is the bound value inside a guard
    $[_ < 10] >> "small",
    _         >> "large"
  );
}

int classify(int x) {
  return match(x) | on(
    lit(0) >> 0,   // runtime literal
    val<1> >> 1,   // compile-time literal (eligible for static dispatch)
    _      >> -1   // required fallback
  );
}

int greet(const std::string &s) {
  return match(s) | on(
    lit_ci("hello") >> 1,   // ASCII case-insensitive
    _               >> 0
  );
}
```

### Structs

`has<&T::member...>` describes structure; wrap it in `$(...)` to pass the
listed members to the handler, in the order you list them.

```cpp
struct Point { int x; int y; };

int magnitude2(const Point &p) {
  return match(p) | on(
    $(has<&Point::x, &Point::y>) >> [](int x, int y) {
      return x * x + y * y;
    },
    _ >> 0
  );
}
```

### Named placeholders in guards

A `has<...>` pattern is tuple-like, so a C++17 structured binding gives its
members names you can use directly in guard expressions:

```cpp
struct Point { int x; int y; };

constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;   // placeholders, not values

bool on_circle_radius5(const Point &p) {
  return match(p) | on(
    $(pt)[x * x + y * y == 25] >> true,
    _                          >> false
  );
}
```

### `std::variant`

Match an alternative by type with `is<T>` or by index with `alt<I>`; wrap it
in `$(...)` to pass the held value to the handler (see the example at the top).

```cpp
using Value = std::variant<int, std::string>;

const char *kind(const Value &v) {
  return match(v) | on(
    is<int> >> "int",      // by type
    alt<1>  >> "string",   // by index
    _       >> "other"
  );
}
```

### Combinators

```cpp
const char *char_kind(char c) {
  return match(c) | on(
    any(lit('a'), lit('e'), lit('i'), lit('o'), lit('u')) >> "vowel",
    (lit(' ') || lit('\n')) >> "whitespace",   // same as any(...)
    !lit('\0')              >> "other",        // same as neg(...)
    _                       >> "end"
  );
}
```

`>>` binds tighter than `||` and `&&`, so combined patterns need
parentheses; unary `!` does not.

### Putting it together: event dispatch

Variant dispatch, structural binding, named-field guards, and a combinator in
one match:

```cpp
struct Click  { int x, y; };
struct Key    { char code; bool ctrl; };
struct Resize { int w, h; };
struct Quit   {};
using Event = std::variant<Click, Key, Resize, Quit>;

constexpr auto click = has<&Click::x, &Click::y>;
constexpr auto key   = has<&Key::code, &Key::ctrl>;
auto [x, y]       = click;   // name the fields once,
auto [code, ctrl] = key;     // then use them in guards

std::string handle(const Event &e) {
  return match(e) | on(
    is<Click>($(click))[x < 0 || y < 0]         >> "click outside",
    is<Click>($(click)) >> [](int cx, int cy) {
      return "click at " + std::to_string(cx) + "," + std::to_string(cy);
    },
    is<Key>($(key))[ctrl == true && code == 'q'] >> "quit",
    (is<Resize> || is<Quit>)                     >> "window event",
    _                                            >> "ignored"
  );
}
```

### Cached case packs for hot paths

`PTN_ON(...)` wraps `static_on(...)` so the matcher is built once instead
of on every call:

```cpp
int fast_classify(int x) {
  return match(x) | PTN_ON(
    val<1> >> 1,
    val<2> >> 2,
    _      >> 0
  );
}
```

`PTN_ON` uses a captureless lambda, so function-local structured-binding
names are not visible inside it. Declare the pattern and its names at
namespace scope (as above), use `static` declarations in C++20, or keep the
deprecated `PTN_BIND` in C++17. See
[Scope and cached matchers](https://patternia.tech/api/#scope-and-cached-matchers).

## Core Concepts

| Construct | Meaning |
|---|---|
| `match(subject)` | Starts a match on an lvalue subject. |
| `on(cases...)` | Ordered case list; the last case must be the `_` fallback. |
| `pattern >> handler` | One case. The handler is a value or a callable. |
| `_` | Wildcard / fallback, and the single-value guard placeholder. |
| `lit(v)`, `val<V>`, `lit_ci(s)` | Runtime, compile-time, and ASCII case-insensitive literals. |
| `$`, `$(p)` | Bind the whole subject, or the values exposed by subpattern `p`. |
| `p[guard]` | Run `guard` after `p` matches and binds; failure falls through to the next case. |
| `rng(lo, hi[, mode])` | Range guard helper. |
| `has<&T::m...>` | Structural pattern over the listed data members. |
| `is<T>`, `alt<I>` | `std::variant` alternative by type or by index. |
| `any`, `all`, `neg` | Pattern combinators (`\|\|`, `&&`, `!`). |
| `static_on`, `PTN_ON` | Cached case packs. |

The full reference is in the [API documentation](https://patternia.tech/api/).

## Requirements

- C++17 or later (the CMake target requires `cxx_std_17`). Floating-point
  `val<V>` literals need C++20.
- A compiler at or above the minimums enforced by the CMake configure check
  (`cmake/CheckCompilerVersion.cmake`):

  | Compiler | Minimum |
  |---|---|
  | GCC | 11 |
  | Clang | 12 |
  | AppleClang | 14 |
  | MSVC | 19.30 (Visual Studio 2022) |
  | clang-cl | 18, with an MSVC 19.30+ toolset |

CI builds and tests GCC 11/13 and Clang 14/16/18 on Linux (C++17 through
C++26), and MSVC and clang-cl on Windows (C++20 through C++26).

## Installation

Patternia is header-only and has no external dependencies. Every method
below provides the `patternia::patternia` CMake target.

### vcpkg

```bash
vcpkg install patternia
```

```cmake
find_package(patternia CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE patternia::patternia)
```

### CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(patternia
  GIT_REPOSITORY https://github.com/sentomk/patternia.git
  GIT_TAG v0.9.5
)
FetchContent_MakeAvailable(patternia)

target_link_libraries(your_target PRIVATE patternia::patternia)
```

### Install and `find_package`

```bash
git clone https://github.com/sentomk/patternia.git
cmake -S patternia -B patternia/build -DPTN_BUILD_TESTS=OFF \
  -DPTN_BUILD_BENCHMARKS=OFF -DPTN_BUILD_SAMPLES=OFF
cmake --install patternia/build --prefix /path/to/install
```

```cmake
find_package(patternia CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE patternia::patternia)
```

### Without a package manager

Add the repository as a subdirectory (for example a Git submodule) with
`add_subdirectory(extern/patternia)`, or simply put `include/` on your
include path and `#include <ptn/patternia.hpp>`.

See the [Installation Guide](https://patternia.tech/guide/installation/)
for CPM.cmake, vcpkg manifest mode, and more detail.

## Building the Tests

```bash
cmake -S . -B build -DPTN_BUILD_TESTS=ON
cmake --build build --target ptn_tests
ctest --test-dir build --output-on-failure
```

Other CMake options: `PTN_BUILD_SAMPLES`, `PTN_BUILD_BENCHMARKS`,
`PTN_BUILD_FUZZ` (Clang only), `PTN_INSTALL`, and
`PTN_SKIP_COMPILER_CHECK`. Tests, samples, and benchmarks are on by
default only when Patternia is the top-level project.

## Performance

<p align="center">
  <img src="https://raw.githubusercontent.com/sentomk/patternia/bench-charts/latest.png" alt="Patternia benchmark comparison" width="90%">
</p>

<p align="center"><em>Patternia gap map across key scenarios. Each row is normalized to the fastest implementation in that scenario.</em></p>

See the [Performance Notes](https://patternia.tech/performance/) for full
reports, methodology, and per-version results, and the
[Performance Tuning guide](https://github.com/SentoMK/patternia/blob/main/docs/guide/performance-tuning.md)
for hot-path advice.

## Documentation

- [Getting Started](https://patternia.tech/guide/getting-started/)
- [Installation](https://patternia.tech/guide/installation/)
- [API Reference](https://patternia.tech/api/)
- [Design Overview](https://patternia.tech/design-overview/)
- [Tutorials](https://patternia.tech/tutorials/from-control-flow/)
- [Performance Notes](https://patternia.tech/performance/)
- [Release Notes](https://patternia.tech/changelog/releases/)
- [Samples](samples/)

## Contributing

Bug reports, feature proposals, and pull requests are welcome. Please read
[CONTRIBUTING.md](CONTRIBUTING.md) first: `main` is protected, pull
requests carry a single Conventional Commits commit, and formatting is
checked with `clang-format`. Coding agents should also read
[AGENTS.md](AGENTS.md). This project is governed by the
[Code of Conduct](CODE_OF_CONDUCT.md); report security issues as described
in [SECURITY.md](SECURITY.md).

## License

Patternia is released under the [MIT License](LICENSE).
