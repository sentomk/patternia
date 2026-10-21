# Patternia API Reference

## Overview

Patternia exposes one public matching shape:

```cpp
match(subject) | on(
  case_1,
  case_2,
  _ >> fallback
)
```

The pipeline is immediate.
There is no deferred builder stage.

---

## `match(subject)` {#matchsubject}

`match(subject)` creates the evaluation context for one subject value.

```cpp
int x = 7;
auto r = match(x) | on(
  lit(7) >> 1,
  _ >> 0
);
```

Rules:

- `subject` must be an lvalue.
- Cases are evaluated in source order.
- Matching uses first-match-wins semantics.

---

## `on(case1, case2, ...)` {#oncases}

`on(...)` groups the case list consumed by `match(subject)`.

```cpp
match(x) | on(
  lit(1) >> "one",
  lit(2) >> "two",
  _ >> "other"
);
```

Rules:

- The last case must be a wildcard fallback.
- Each entry must be a `pattern >> handler` case expression.
- Unreachable wildcard/alt ordering errors are diagnosed at compile time.

---

## `pattern >> handler` {#pattern-handler}

This is the core case-expression form.

```cpp
lit(1) >> 42
$(is<int>) >> [](int v) { return v * 2; }
$(has<&Point::x, &Point::y>) >> [](int x, int y) { return x + y; }
```

Handler forms:

- `pattern >> value`
- `pattern >> callable`

Bindings are determined entirely by the pattern.

---

## Wildcard `_` {#_-wildcard}

`_` is the public wildcard and fallback pattern.

```cpp
match(x) | on(
  lit(1) >> "one",
  _ >> "other"
);
```

The wildcard does not bind values by itself.

---

## Literal Patterns {#literal-patterns}

### `lit(value)`

Runtime literal match.

```cpp
match(x) | on(
  lit(5) >> 42,
  _ >> -1
);
```

### `val<value>`

Compile-time literal match.
Useful when the literal is known at compile time and the lowering engine can
consider static dispatch.

```cpp
match(x) | on(
  val<1> >> 1,
  val<2> >> 2,
  _ >> 0
);
```

### `lit_ci(value)`

Runtime ASCII case-insensitive string match.

```cpp
match(s) | on(
  lit_ci("hello") >> 1,
  _ >> 0
);
```

---

## Binding Patterns {#binding-patterns}

### `$` and `$(...)`

Patternia keeps binding explicit.

```cpp
$               // bind the whole subject
$(subpattern)   // bind under a subpattern
```

Examples:

```cpp
match(x) | on(
  $ >> [](int v) { return v; },
  _ >> 0
);
```

```cpp
match(v) | on(
  $(is<std::string>) >> [](const std::string &s) {
    return s.size();
  },
  _ >> 0
);
```

Binding rules:

- No pattern binds implicitly.
- Handler parameters follow binding order.
- Patterns that do not bind produce zero-argument handlers.

---

## Guard Attachment `[]` {#guard-attachment}

Attach a guard to a binding pattern with `pattern[guard]`.

```cpp
match(x) | on(
  $[_ > 0 && _ < 10] >> "small",
  _ >> "other"
);
```

Guard evaluation order:

1. Match the pattern.
2. Bind values.
3. Evaluate the guard.
4. Invoke the handler if the guard passes.

A guard failure only rejects the current case.

---

## Guard Helpers {#guard-helpers}

### `_`

Placeholder alias for a single bound value.

```cpp
$[_ > 5]
```

Use `_` when one binding is enough and the predicate reads clearly without an
explicit name.

### Structured-binding member names

A `has<...>` pattern is tuple-like. Prefer a C++17 structured binding to name
its member placeholders without a macro:

```cpp
constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;

$(pt)[x * x + y * y == 25]
```

There is one name per non-ignored member, in the order listed in `has<...>`;
`_ign` slots do not get names. The names are placeholders, not values extracted
from a subject. `$(pt)` still controls which values reach the handler.

Inside a guard, names resolve to the position of their member in the `has<>`
member list at compile time, so they follow members, not positions — the order
of member pointers in a subsequent pattern does not matter:

```cpp
$(has<&Point::y, &Point::x>)[x == 3 && y == 4]  // x is still .x
```

A name whose member is not listed in `has<>` fails a static_assert. Member
names are only valid in guards attached to `has<...>`; using them on a
non-structural pattern is a compile-time error.

#### Scope and cached matchers

Function-local `auto [x, y] = pt;` works with `on(...)`, but not with
`PTN_ON(...)`: the macro uses a captureless lambda, which cannot access these
automatic bindings. For cached matchers, choose one of these options:

- Declare the pattern and its structured-binding names at namespace scope.
- In C++20 or later, give both the pattern and the bindings static storage
    inside the function:

    ```cpp
    static constexpr auto pt = has<&Point::x, &Point::y>;
    static auto [x, y] = pt;
    ```

- In C++17, keep `PTN_BIND` for function-local names used inside `PTN_ON`, or
  use `on(...)` instead. Any local pattern passed to `PTN_ON` must also be
  static; alternatively, write `has<...>` directly inside the macro.

### `PTN_BIND(Type, names...)` (deprecated)

`PTN_BIND` remains supported for compatibility, but prefer structured-binding
member names where the scope rules above permit them. It declares
`static constexpr member_t<&Type::name>` placeholders for one to ten
non-static data members:

```cpp
PTN_BIND(Point, x, y);

$(has<&Point::x, &Point::y>)[x * x + y * y == 25]
```

Declarations are valid at namespace or block scope. Their static storage
duration allows block-scope names inside `PTN_ON` even in C++17. A misspelled
member name fails at the `PTN_BIND` line.

### `rng(lo, hi, mode)`

Range helper for single-bound-value guards.

```cpp
$[rng(0, 10)]
$[rng(0, 10, pat::mod::open)]
```

Use callables for domain logic that does not read naturally as `_` or a short
named-placeholder expression.

### Migration from positional guard APIs

The guard surface now uses `_` for one bound value and structured-binding
member names for structural guards. The former `__`, `_0`, `arg<N>`,
`PTN_LET`, and `PTN_WHERE` spellings have been removed rather than retained as aliases.

```cpp
// Single value
$[_ > 0]

// Multiple structural members
constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;
$(pt)[x < y]
```

---

## Structural Matching `has<&T::member...>` {#structural-matching}

`has<>` describes structure.
Wrap it with `$(...)` to extract values.

```cpp
struct Point {
  int x;
  int y;
};

match(p) | on(
  $(has<&Point::x, &Point::y>) >> [](int x, int y) {
    return x + y;
  },
  _ >> 0
);
```

Properties:

- Member order is explicit and stable.
- Unlisted members are ignored.
- Validation happens at compile time.

---

## Variant Matching `is<T>` and `alt<I>` {#variant-matching}

### `is<T>`

Type-based `std::variant` match.

```cpp
match(v) | on(
  is<int> >> "int",
  $(is<std::string>) >> [](const std::string &s) {
    return "str:" + s;
  },
  _ >> [] { return std::string("other"); }
);
```

### `alt<I>`

Index-based `std::variant` match.

```cpp
match(v) | on(
  alt<0> >> "first",
  alt<1> >> "second",
  _ >> "other"
);
```

Rules:

- `is<T>` requires `T` to appear exactly once.
- `alt<I>` requires `I` to be in range.
- Use `$(...)` when you want the alternative value bound into the handler.

---

## Pattern Combinators {#pattern-combinators}

### `any(ps...)`

Matches when at least one sub-pattern matches (OR semantics). Short-circuits
on the first matching sub-pattern.

```cpp
match(x) | on(
  any(val<1>, val<2>, val<3>) >> "1, 2, or 3",
  _ >> "other"
);
```

Properties:

- Non-binding: handlers receive zero arguments.
- Sub-patterns are evaluated left-to-right; evaluation stops at the first
  match.
- Requires at least one sub-pattern; every argument must be a pattern object.
- Operator sugar: `(a || b)` is equivalent to `any(a, b)`. Note that `>>`
  binds tighter than `||`, so parenthesize: `(lit(1) || lit(2)) >> handler`.

### `all(ps...)`

Matches only when every sub-pattern matches (AND semantics). Short-circuits
on the first mismatch.

```cpp
match(x) | on(
  all(any(val<1>, val<2>), val<2>) >> "2",
  _ >> "other"
);
```

Properties:

- Non-binding: handlers receive zero arguments.
- Sub-patterns are evaluated left-to-right; evaluation stops at the first
  mismatch.
- Requires at least one sub-pattern; every argument must be a pattern object.
- Operator sugar: `(a && b)` is equivalent to `all(a, b)`. Note that `>>`
  binds tighter than `&&`, so parenthesize: `(p && q) >> handler`.

### `neg(p)`

Negates the match result of a sub-pattern. `neg(p)` matches when `p` does
not match, and vice versa.

```cpp
match(x) | on(
  neg(val<0>) >> "non-zero",
  _ >> "zero"
);
```

Properties:

- Non-binding: handlers receive zero arguments.
- Accepts exactly one sub-pattern (no zero- or multi-argument form).
- `neg(neg(p))` restores the original match behavior (double negation cancels).
- Operator sugar: `!p` is equivalent to `neg(p)` and needs no parentheses:
  `!val<200> >> "error"`.

The pattern-level operators only accept pattern operands, so they never
collide with the guard-level `&&` / `||` (which keep their `pred_and` /
`pred_or` meaning inside `[...]` guards).

---

## Cached Case Packs {#cached-case-packs}

### `static_on(...)`

Cache a stateless `on(...)` factory.

```cpp
match(x) | static_on([] {
  return on(
    val<1> >> 1,
    val<2> >> 2,
    _ >> 0
  );
});
```

### `PTN_ON(...)`

Convenience macro over `static_on(...)`.

```cpp
match(x) | PTN_ON(
  val<1> >> 1,
  val<2> >> 2,
  _ >> 0
);
```

The factory must be stateless.

---

## Namespace Summary {#namespace-summary}

The public surface is available through `namespace ptn`, plus the macros listed
below:

- `match`
- `on`
- `lit`, `val`, `lit_ci`
- `$`
- `_`, `rng`
- `has`
- `is`, `alt`
- `any`, `all`, `neg`
- `PTN_ON` (macro)
- `PTN_BIND` (deprecated compatibility macro)

---

## Minimal Example

```cpp
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;

  int x = 2;
  int r = match(x) | on(
    lit(1) >> 10,
    lit(2) >> 20,
    _ >> 0
  );

  return r == 20 ? 0 : 1;
}
```
