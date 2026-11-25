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
[![Docs](https://img.shields.io/badge/docs-online-blue?style=flat-square)](https://patternia.cc)

</div>

<br>

**Patternia** is a header-only pattern matching library for C++17 and later.
Match literals, `std::variant` alternatives and struct members, bind the
values you need, guard them, and compute the result, all in one
expression:

```cpp
#include <ptn/patternia.hpp>
#include <string>
#include <variant>
using namespace ptn;

using Value = std::variant<int, std::string>;

std::string describe(const Value &v) {
  return match(v) | on(
    is<int>(0)                  >> "zero",
    $(is<int>)[_ < 0]           >> [](int n) { return "negative " + std::to_string(n); },
    $(is<int>)                  >> [](int n) { return "positive " + std::to_string(n); },
    $(is<std::string>)[_ != ""] >> [](const std::string &s) { return "text: " + s; },
    _                           >> "empty"
  );
}
```

Cases are tried in order and the first match wins. `$(...)` binds the
matched value and passes it to the handler, `[...]` is a guard that must
also hold, and `_` matches anything.

The match is planned at compile time. This one switches on the
variant's active alternative first and then tries only that
alternative's cases; a list of integer literals compiles to the same
code as a hand-written `switch`.

## Highlights

- **One DSL for many shapes**: literals (runtime, compile-time and ASCII
  case-insensitive), struct members, and `std::variant` alternatives by
  type or by index. Plain values such as `0`, `'a'` or `"hi"` can stand
  wherever a pattern is expected.
- **Expressions all the way**: `match(x) | on(...)` returns a value.
  Guards (`$[_ > 0 && _ < 10]`) and handlers (`$ >> _ * 2`) can be
  written as expressions over the bound values instead of lambdas.
- **Explicit binding**: nothing binds implicitly. `$` binds the subject,
  `$(pattern)` binds what a subpattern exposes, and handler parameters
  follow binding order. Structured bindings give struct members names
  you can use in guards and handlers.
- **Owning matches**: `match(std::move(x))` moves the subject into the
  match. Patterns and guards only look at it; the selected handler can
  take it over, so move-only types such as `std::unique_ptr` work.
- **Compile-time checks**: a missing fallback, unreachable cases,
  out-of-range `has<...>` indices, invalid member names and malformed
  cases are diagnosed at compile time.
- **Costs what hand-written code costs**: no RTTI, no virtual dispatch,
  no heap allocation. Literal and variant matches compile to a `switch`
  on GCC and Clang, and a codegen check in CI keeps them that way.
- **No dependencies**: just headers. Packaged for vcpkg and exported as
  a CMake target.

## Performance

A match costs what the same branches cost when written by hand. The case
list is planned at compile time from the subject's type and the patterns:

| Case list | Compiles to |
|---|---|
| `val<N>` literals close together, on an integer | A `switch` on the subject's offset: one compare, then a table |
| `lit(v)` literals, on an integer or enum | Comparisons that pick a case index, then the same `switch` |
| `std::variant` alternatives, any mix of cases | A `switch` on `index()`, then only that alternative's cases (an owned variant needs five or more alternatives) |
| Anything else | The cases in order, like an `if` chain |

Measured time per match in ns (v0.9.6, Apple M5 Pro, Apple Clang 17,
`-O2`; lower is better):

| Scenario | Patternia | Hand-written | `std::visit` |
|---|---:|---:|---:|
| 64 dense integer literals | 0.92 | 0.92 (`switch`) | — |
| 32-alternative variant | 0.90 | 0.91 (`switch` on `index()`) | — |
| 16-alternative event variant | 1.31 | 1.32 | 2.19 |
| Protocol router (variant with guards) | 0.89 | 0.83 (`if` chain) | 1.00 |
| Packet classification (struct members, guards) | 1.15 | 1.56 (`switch`) | — |

`scripts/check_codegen.py` compiles representative matches in CI and fails
when one makes a call, touches the stack or grows past a hand-written
reference. The [Performance Tuning guide](https://patternia.cc/guide/performance-tuning/)
explains which matches take which path; the
[Performance Notes](https://patternia.cc/performance/) have per-release
measurements, GCC results and methods, and the
[benchmark report](https://github.com/sentomk/patternia/blob/bench-charts/latest.md)
is updated by CI on every push to `main`.

## Quick Tour

All examples assume `#include <ptn/patternia.hpp>`, the standard headers
they use, and `using namespace ptn;`.

### Literals, guards and expressions

```cpp
const char *bucket(int x) {
  return match(x) | on(
    lit(0)    >> "zero",
    $[_ < 0]  >> "negative",   // `_` is the bound value inside a guard
    $[_ < 10] >> "small",
    _         >> "large"
  );
}

int collatz_step(int n) {
  return match(n) | on(
    $[_ % 2 == 0] >> _ / 2,
    $             >> (_ * 3 + 1)   // `$` alone can be the last case
  );
}

int operand_count(int opcode) {
  return match(opcode) | on(
    val<0x00> >> 0,   // compile-time literals close together
    val<0x01> >> 2,   // compile to a jump table
    val<0x02> >> 2,
    val<0x03> >> 1,
    val<0x05> >> 1,
    _         >> -1
  );
}

bool is_greeting(const std::string &s) {
  return match(s) | on(
    lit_ci("hello") >> true,   // ASCII case-insensitive
    _               >> false
  );
}
```

`lit(v)` compares against a value known at run time; `val<V>` puts the
value in the type, so the planner can build a table from it.

In a handler, comparisons need parentheses (`$ >> (_ > 0)`), because C++
gives `>>` higher precedence. GCC's `-Wall` also asks for them around
`+` and `-`, as in `(_ * 3 + 1)` above.

### Structs

`has<&T::member...>` describes structure. Wrap it in `$(...)` to pass the
listed members to the handler, in the order you list them:

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

A `has<...>` pattern is tuple-like, so a C++17 structured binding gives
its members names. Use them in guards and in handlers:

```cpp
constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;   // placeholders, not values

int quadrant(const Point &p) {
  return match(p) | on(
    $(pt)[x == 0 || y == 0] >> 0,
    $(pt)[x > 0 && y > 0]   >> 1,
    $(pt)[x < 0 && y > 0]   >> 2,
    $(pt)[x < 0]            >> 3,
    _                       >> 4
  );
}

int manhattan(const Point &p) {
  return match(p) | on(
    $(pt)[x >= 0 && y >= 0] >> (x + y),
    $(pt)                   >> [](int px, int py) {
      return (px < 0 ? -px : px) + (py < 0 ? -py : py);
    },
    _ >> 0
  );
}
```

### `std::variant`

Match an alternative by type with `is<T>` or by index with `alt<I>`. Wrap
it in `$(...)` to pass the held value to the handler, and give it a
subpattern to look inside:

```cpp
using Value = std::variant<int, std::string>;

const char *kind(const Value &v) {
  return match(v) | on(
    is<int>(0)         >> "int zero",
    is<int>            >> "int",      // by type
    alt<1>("")         >> "empty string",
    alt<1>             >> "string",   // by index
    _                  >> "valueless"
  );
}

int doubled(const Value &v) {
  return match(v) | on(
    $(is<int>) >> _ * 2,
    _          >> 0
  );
}
```

A variant match switches on `index()` first and then tries only the cases
that can match that alternative.

### Combinators

```cpp
const char *char_kind(char c) {
  return match(c) | on(
    any('a', 'e', 'i', 'o', 'u') >> "vowel",
    (lit(' ') || '\n')           >> "whitespace",   // same as any(...)
    !lit('\0')                   >> "other",        // same as neg(...)
    _                            >> "end"
  );
}
```

`>>` binds tighter than `||` and `&&`, so combined patterns need
parentheses; unary `!` does not. `!` needs a pattern operand
(`!lit('\0')`, or `neg('\0')`), because `!` on a plain `char` is the
built-in operator.

### Owning the subject

An lvalue subject is borrowed: patterns, guards and handlers see it
through `const` references. An rvalue is owned: `match(std::move(x))` or
`match(make_x())` moves it into the match. Guards still only look at it,
a rejected case leaves it intact, and the handler of the selected case
receives it by value:

```cpp
struct Job { int priority; };
void run(std::unique_ptr<Job> job);
void queue(std::unique_ptr<Job> job);

void schedule(std::unique_ptr<Job> job) {
  match(std::move(job)) | on(
    $[([](const auto &j) { return j && j->priority > 5; })]
      >> [](std::unique_ptr<Job> j) { run(std::move(j)); },
    $ >> [](std::unique_ptr<Job> j) { queue(std::move(j)); }
  );
}
```

The same holds inside variants (`$(is<T>)` hands over the alternative)
and struct members. Owned matches use the same compiled dispatch as
borrowed ones, except that an owned variant with four or fewer
alternatives is matched in source order.

### Putting it together: event dispatch

Variant dispatch, structural binding, named-field guards and a combinator
in one match:

```cpp
struct Click  { int x, y; };
struct Key    { char code; bool ctrl; };
struct Resize { int w, h; };
struct Quit   {};
using Event = std::variant<Click, Key, Resize, Quit>;

constexpr auto click = has<&Click::x, &Click::y>;
constexpr auto key   = has<&Key::code, &Key::ctrl>;
auto [cx, cy]     = click;   // name the fields once,
auto [code, ctrl] = key;     // then use them in guards

std::string handle(const Event &e) {
  return match(e) | on(
    is<Click>($(click))[cx < 0 || cy < 0]       >> "click outside",
    is<Click>($(click)) >> [](int x, int y) {
      return "click at " + std::to_string(x) + "," + std::to_string(y);
    },
    is<Key>($(key))[ctrl == true && code == 'q'] >> "quit",
    (is<Resize> || is<Quit>)                     >> "window event",
    _                                            >> "ignored"
  );
}
```

### Reusing case lists

A case list is a value. Keep one in a `constexpr` (or `static const`)
variable to build it once and reuse it:

```cpp
constexpr auto parity = on(val<0> >> "even", val<1> >> "odd", _ >> "?");

const char *parity_of(int n) { return match(n % 2) | parity; }
```

`PTN_ON(...)` does the same inline, caching the list in a function-local
static on the first call:

```cpp
int fast_classify(int x) {
  return match(x) | PTN_ON(
    val<1> >> 1,
    val<2> >> 2,
    _      >> 0
  );
}
```

Caching matters when building the cases does work, such as constructing a
`std::string`. With integers and captureless lambdas, the compiler folds
`on(...)` away on its own.

`PTN_ON` uses a captureless lambda, so function-local structured-binding
names are not visible inside it. Declare the pattern and its names at
namespace scope (as above), use `static` declarations in C++20, or keep the
deprecated `PTN_BIND` in C++17. See
[Scope and cached matchers](https://patternia.cc/api/#scope-and-cached-matchers).

## Core Concepts

| Construct | Meaning |
|---|---|
| `match(subject)` | Starts a match. An lvalue subject is borrowed; an rvalue is moved in and owned. |
| `on(cases...)` | Ordered case list. The last case must be `_` or an unguarded `$`. |
| `pattern >> handler` | One case. The handler is a value, a callable, or an expression over the bound values. |
| `_` | Wildcard and fallback; inside guards and expression handlers, the first bound value. |
| `lit(v)`, `val<V>`, `lit_ci(s)` | Runtime, compile-time and ASCII case-insensitive literals. A plain value inside a pattern means `lit(value)`. |
| `$`, `$(p)` | Bind the whole subject, or the values exposed by subpattern `p`. |
| `p[guard]` | Run `guard` after `p` matches and binds; failure falls through to the next case. |
| `rng(lo, hi[, mode])` | Range guard helper. |
| `has<&T::m...>` | Structural pattern over the listed data members; tuple-like for named placeholders. |
| `is<T>`, `alt<I>` | `std::variant` alternative by type or by index, optionally with a subpattern. |
| `any`, `all`, `neg` | Pattern combinators (`\|\|`, `&&`, `!`). |
| `static_on`, `PTN_ON` | Cached case packs. |

The full reference is in the [API documentation](https://patternia.cc/api/).

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
  GIT_TAG v0.9.7
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

### Single header

`single_include/ptn/patternia.hpp` is the whole library in one file:
download it and `#include "patternia.hpp"`, nothing else needed.

```bash
curl -LO https://raw.githubusercontent.com/sentomk/patternia/main/single_include/ptn/patternia.hpp
```

### Without a package manager

Add the repository as a subdirectory (for example a Git submodule) with
`add_subdirectory(extern/patternia)`, or simply put `include/` on your
include path and `#include <ptn/patternia.hpp>`.

See the [Installation Guide](https://patternia.cc/guide/installation/)
for CPM.cmake, vcpkg manifest mode, and more detail.

## Building the Tests

```bash
cmake -S . -B build -DPTN_BUILD_TESTS=ON
cmake --build build --target ptn_tests
ctest --test-dir build --output-on-failure
```

Other CMake options: `PTN_BUILD_SAMPLES`, `PTN_BUILD_BENCHMARKS`,
`PTN_BUILD_FUZZ` (Clang only), `PTN_INSTALL`, `PTN_DEV_INDEX` (local IDE
indexing target for top-level builds, not installed; on by default), and
`PTN_SKIP_COMPILER_CHECK`. Tests, samples, and benchmarks are on by
default only when Patternia is the top-level project.

## Documentation

- [Getting Started](https://patternia.cc/guide/getting-started/)
- [Installation](https://patternia.cc/guide/installation/)
- [API Reference](https://patternia.cc/api/)
- [Design Overview](https://patternia.cc/design-overview/)
- [Tutorials](https://patternia.cc/tutorials/from-control-flow/)
- [Performance Tuning](https://patternia.cc/guide/performance-tuning/)
- [Performance Notes](https://patternia.cc/performance/)
- [Release Notes](https://patternia.cc/changelog/releases/)
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
