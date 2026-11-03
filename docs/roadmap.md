---
hide:
  - navigation
---

# Roadmap

This page lists the features planned for upcoming Patternia releases,
grouped by the version they are expected to ship in. It describes
direction, not a promise: scope, names and syntax can still change
before a release, and a feature can move to a later version.

Syntax shown for unreleased features is the current design. It is not
available in any release yet.

Each version has one status:

| Status | Meaning |
|---|---|
| **Released** | Shipped. See the linked release note. |
| **In progress** | Being designed or implemented now. |
| **Planned** | Scheduled for this version, work not started. |
| **Exploring** | Not scheduled. Needs a design decision or user demand first. |

Feedback on any item is welcome in
[GitHub issues](https://github.com/sentomk/patternia/issues).

---

## v0.10 — Shorter cases and variant exhaustiveness { data-toc-label="v0.10" }

**Status:** Planned

The goal of this release is to remove the boilerplate that most
matches carry today: the unreachable `_` at the end of a variant match,
single-expression lambdas, and `lit(...)` inside other patterns.

### Exhaustiveness checking for `std::variant`

A match on a `std::variant` no longer needs a trailing `_` when every
alternative is covered. A missing alternative becomes a compile-time
error that names it:

```cpp
using Shape = std::variant<Circle, Square, Triangle>;

double area(const Shape &s) {
  return match(s) | on(
    $(is<Circle>)   >> [](const Circle &c) { return c.area(); },
    $(is<Square>)   >> [](const Square &q) { return q.area(); },
    $(is<Triangle>) >> [](const Triangle &t) { return t.area(); }
  );   // no `_` needed
}
```

```text
[Patternia.on]: non-exhaustive match on std::variant<Circle, Square, Triangle>.
Missing alternatives: Triangle.
Tip: add the missing cases, or end the list with '_ >> handler'.
```

- Only unguarded `is<T>` and `alt<I>` cases count as covering an
  alternative. A guard or a refutable subpattern never does.
- A `bool` subject is covered by `val<true>` and `val<false>`.
- A trailing `_` after a complete list stays allowed, so existing code
  keeps compiling.

### Binding patterns as the fallback

`$` and other patterns that match every value can end a case list on
their own, instead of being followed by an unreachable `_`:

```cpp
match(roll) | on(
  lit(3) >> add_fancy_hat,
  lit(7) >> remove_fancy_hat,
  $      >> [](int r) { move_player(r); }   // already the fallback
);
```

### Expression handlers

The right-hand side of `>>` accepts the same expressions as guards, so a
handler that only computes a value no longer needs a lambda:

```cpp
constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;

match(p) | on(
  $(pt)[x > 0] >> x - y,
  $(pt)        >> x * x + y * y,
  _            >> 0
);

match(n) | on(
  $(is<int>) >> _ * 2 + 1,
  _          >> 0
);
```

The first version covers the arithmetic, comparison, logical and unary
operators that guards already support. Calling functions inside an
expression handler is out of scope; use a lambda for that.

### Plain values inside patterns

Wherever a pattern is expected as an argument, a plain value is treated
as `lit(value)`. This extends the existing `0 >> handler` shorthand to
nested positions:

```cpp
// today
any(lit('a'), lit('e'), lit('i')) >> "vowel",
is<int>(lit(0))                   >> "int zero",

// v0.10
any('a', 'e', 'i') >> "vowel",
is<int>(0)         >> "int zero",
(lit(' ') || '\n') >> "whitespace",
```

It applies to the arguments of `any`, `all` and `neg`, to the
subpatterns of `is<T>(...)` and `alt<I>(...)`, and to the right
operand of `||` and `&&`. A plain value is a runtime literal; use
`val<V>` where static dispatch matters.

### Fixes and diagnostics

- `$(pred(...))` binds the matched value, like `$(is<T>)` does.
- When both sides of `>>` are built-in types (`0 >> 1`, `0 >> func`),
  C++ applies the built-in shift instead of Patternia's operator. The
  error now says so and suggests `lit(0) >> ...`, instead of reporting
  that the case was not created with `>>`.

---

## v0.11 — Nested and multi-subject matching { data-toc-label="v0.11" }

**Status:** Planned

The goal of this release is to match on the shape of data: members
with their own patterns, tuple elements, optional values, and several
subjects at once. The new patterns ship as an experimental API for one
minor version before their syntax is frozen.

### Patterns on members and elements

`has<...>` takes one subpattern per listed member, and `at<...>`
projects a single member or element before matching it:

```cpp
match(p) | on(
  has<&Point::x, &Point::y>(0, 0) >> "origin",
  has<&Point::x, &Point::y>(_, 0) >> "on x axis",
  at<&Point::y>(0)                >> "on x axis",   // same as above
  _                               >> "elsewhere"
);
```

`at<I>(p)` does the same for the `I`-th element of a tuple, pair or
array. Patterns nest to any depth, and the bindings of the
subpatterns are concatenated in the order they are written.

### Tuple patterns and multiple subjects

`tup(...)` matches a tuple-like subject element by element, and
`match(a, b, ...)` matches several subjects together:

```cpp
for (int i = 1; i <= 100; ++i) {
  match(i % 3, i % 5) | on(
    tup(0, 0) >> [] { std::puts("fizzbuzz"); },
    tup(0, _) >> [] { std::puts("fizz"); },
    tup(_, 0) >> [] { std::puts("buzz"); },
    _         >> [i] { std::printf("%d\n", i); }
  );
}
```

`match` also accepts rvalue subjects, such as `match(make())` and
`match(i % 3, i % 5)`, which today must be lvalues. A temporary subject
is stored by value for the duration of the match.

### `some` / `none`

Patterns for the two states of `std::optional`, raw pointers and smart
pointers:

```cpp
std::optional<int> v = find(id);

match(v) | on(
  some(0)        >> 0,
  $(some)[_ < 0] >> [](int x) { return -x; },
  $(some)        >> [](int x) { return x; },
  none           >> -1
);
```

### Names for nested positions

Structured-binding names extend to nested patterns. A name is anchored
to its full path, so the same member appearing twice gets two
different names:

```cpp
constexpr auto start = at<&Line::a>($(has<&Point::x, &Point::y>));
auto [x, y] = start;

match(line) | on(
  start[x * x + y * y == 25] >> "starts on the circle",
  _                          >> "elsewhere"
);
```

### Exhaustiveness for nested and multi-subject matches

Exhaustiveness checking extends to `tup(...)`, `some` / `none` and
nested variants, using a pattern-matrix algorithm. A missing
combination is reported at compile time. For very large matches the
check stops at a fixed limit and asks for `_` instead.

---

## v0.12 — Performance and C++26 reflection { data-toc-label="v0.12" }

**Status:** Planned

### Performance

- Benchmarks for the default and call spellings, variants with 9 to 15
  alternatives, binding patterns, and nested and multi-subject matches.
- Investigate the scenarios that are currently slower than
  `std::visit` or a hand-written `switch`, and optimize the common
  shapes of the new nested patterns.
- Publish the benchmark scripts and method so that "not slower than
  `std::visit`" is reproducible.

### C++26 reflection mode (experimental)

When the compiler supports C++26 static reflection, the same structural
API accepts reflections in place of pointers to members:

```cpp
constexpr auto pt = has<^^Point::x, ^^Point::y>;
auto [x, y] = pt;
```

Reflection mode also enables:

- exhaustiveness checking for enumerations,
- positional destructuring of aggregates without listing members.

Reflection mode is off by default and does not change behavior in
C++17 or C++20.

---

## v1.0 — Stable API { data-toc-label="v1.0" }

**Status:** Planned

- API freeze, with a migration guide from 0.x.
- Removal of the deprecated `PTN_BIND`, or a final decision to keep
  it.
- Full compiler matrix in CI: GCC, Clang and MSVC, including an MSVC
  C++17 configuration and macOS with Apple Clang.
- No benchmarked `std::variant` scenario slower than `std::visit`.

---

## Exploring

Not scheduled for any version. Each needs a design decision or a
concrete use case first.

- **Single-pattern test.** `matches(x, p)` returns `bool` without a
  case list, for use in `if` conditions and `static_assert`.
- **Custom alternative types.** A customization point that lets user
  types such as `Result<T, E>` work with `is<T>` and exhaustiveness
  checking, beyond `std::variant`.
- **`std::expected` patterns.** Value and error states, including
  `expected<T, T>`.
- **Positional aggregate destructuring in C++17.** Field-counting
  techniques limit this to small aggregates without base classes; the
  alternative is to wait for reflection mode.
- **Sequence patterns.** Matching the head and rest of a `vector` or
  `array`.
- **Calls inside expression handlers.** Helpers such as `call(f, e...)`
  and `iif(cond, a, b)` for handlers that are not plain arithmetic.
- **More value types on the left of `>>`.** Accepting any
  equality-comparable type, not only arithmetic, enum and string
  types.

---

## Released

Recent releases. Earlier versions are in the
[release notes](changelog/releases.md).

| Version | Highlights |
|---|---|
| [v0.9.5](changelog/v0.9.5.md) | `has<...>` is tuple-like for macro-free member names; `PTN_BIND` deprecated; bare `is<T>` / `alt<I>` reach static dispatch; handler classification no longer instantiates generic lambdas |
| [v0.9.4](changelog/v0.9.4.md) | `PTN_BIND` member-anchored guard names; operator sugar for `neg`, `any` and `all` |
| [v0.9.3](changelog/v0.9.3.md) | `pred(callable)` predicate pattern; `neg(p)` |
| [v0.9.2](changelog/v0.9.2.md) | `any(ps...)` and `all(ps...)` combinators |
| [v0.9.1](changelog/v0.9.1.md) | `val<V>` static literals, including C++20 floating-point values; canonical `has<...>` |

---

## Design Principles for New API

- Stateless compile-time patterns are variable templates (`is<T>`,
  `alt<I>`, `val<V>`), not function templates that return
  default-constructed objects.
- Function forms are reserved for patterns that take runtime arguments
  or subpatterns (`lit(value)`, `rng(lo, hi)`, `is<T>(p)`).
- Names are short, lowercase, and read naturally in the DSL.
- No new macros. `PTN_ON` is the only macro meant to stay.
- C++17 is the baseline. Newer language features are optional
  improvements and never change C++17 behavior.
- New patterns work on the general matching path first. Static
  dispatch is an optimization and must not change results.
- No heap allocation, RTTI or virtual dispatch in the matching path.

## Considered and Declined

Ideas that were evaluated and intentionally rejected, kept here so
they are not re-proposed without new motivation.

- **Matching class hierarchies with `dynamic_cast`**: conflicts with the
  no-RTTI principle. Match on a `std::variant` instead.
- **Macro syntax imitating a language `match` statement**: contradicts
  the no-new-macros principle.
- **`0 >> 1` without `lit`**: not possible. C++ only allows an
  overloaded operator when at least one operand has class or
  enumeration type, so two built-in operands always use the built-in
  shift.
- **Handlers that return from the enclosing function**: a handler is a
  callable, so `return`, `break` and `continue` inside it cannot leave
  the enclosing function or loop.
- **Guard `operator!`**: intentionally not provided; rewrite the
  comparison instead (e.g. `!(x < y)` becomes `y <= x`).
- **Member-call placeholders** (`_.size() > 3`): not expressible —
  C++ has no `operator.` overloading, so `.size()` cannot become an
  expression node. Use `pred` with a lambda.
- **Chained comparisons** (`1 <= _ <= 10`): breaks predicate
  semantics and produces unreadable diagnostics. Use `rng(lo, hi)`
  with explicit range modes.
- **`PTN_BIND` `Type` validation**: resolved without reflection —
  names now expand to `member_t<&Type::name>`, so member checking
  happens at declaration time (shipped in v0.9.4).
