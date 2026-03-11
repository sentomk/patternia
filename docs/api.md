# Patternia API Reference

## Overview

Patternia exposes one public matching shape:

```cpp
match(subject) | on(
  case_1,
  case_2,
  __ >> fallback
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
  __ >> 0
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
  __ >> "other"
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
$(is<int>()) >> [](int v) { return v * 2; }
$(has<&Point::x, &Point::y>()) >> [](int x, int y) { return x + y; }
```

Handler forms:

- `pattern >> value`
- `pattern >> callable`

Bindings are determined entirely by the pattern.

---

## Wildcard `__` and `_` {#__-wildcard}

`__` is the explicit fallback pattern.
`_` is an alias.

```cpp
match(x) | on(
  lit(1) >> "one",
  __ >> "other"
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
  __ >> -1
);
```

### `lit<value>()`

Compile-time literal match.
Useful when the literal is known at compile time and the lowering engine can
consider static dispatch.

```cpp
match(x) | on(
  lit<1>() >> 1,
  lit<2>() >> 2,
  __ >> 0
);
```

### `lit_ci(value)`

Runtime ASCII case-insensitive string match.

```cpp
match(s) | on(
  lit_ci("hello") >> 1,
  __ >> 0
);
```

---

## Binding Patterns {#binding-patterns}

### `$()` and `bind()`

Patternia keeps binding explicit.

```cpp
$()             // bind the whole subject
$(subpattern)   // bind under a subpattern
bind()          // same idea, lower-level spelling
bind(subpattern)
```

Examples:

```cpp
match(x) | on(
  $() >> [](int v) { return v; },
  __ >> 0
);
```

```cpp
match(v) | on(
  $(is<std::string>()) >> [](const std::string &s) {
    return s.size();
  },
  __ >> 0
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
  bind()[_0 > 0 && _0 < 10] >> "small",
  __ >> "other"
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

### `_0`, `_1`, `_2`, `_3`

Placeholder aliases for the first few bound values.

```cpp
bind()[_0 > 5]
bind(has<&Point::x, &Point::y>())[_0 + _1 == 0]
```

### `arg<N>`

Indexed placeholder for general multi-binding guards.

```cpp
bind(has<&Point::x, &Point::y>())[arg<0> * arg<0> + arg<1> * arg<1> == 25]
```

### `rng(lo, hi, mode)`

Range helper for single-bound-value guards.

```cpp
bind()[rng(0, 10)]
bind()[rng(0, 10, pat::mod::open)]
```

Use callables for domain logic that does not read naturally as a placeholder
expression.

### `PTN_WHERE((names...), expr)`

Use named guard parameters without writing the lambda yourself.
The macro currently supports 1 to 5 names.

```cpp
match(p) | on(
  bind(has<&Point::x, &Point::y>())[PTN_WHERE((x, y), x == y)] >> "diagonal",
  __ >> "other"
);
```

`PTN_WHERE(...)` expands to a guard callable and composes with `&&` / `||`
like other guard predicates.

---

## Structural Matching `has<&T::member...>()` {#structural-matching}

`has<>` describes structure.
Wrap it with `$()` or `bind()` to extract values.

```cpp
struct Point {
  int x;
  int y;
};

match(p) | on(
  $(has<&Point::x, &Point::y>()) >> [](int x, int y) {
    return x + y;
  },
  __ >> 0
);
```

Properties:

- Member order is explicit and stable.
- Unlisted members are ignored.
- Validation happens at compile time.

---

## Variant Matching `is<T>()` and `alt<I>()` {#variant-matching}

### `is<T>()`

Type-based `std::variant` match.

```cpp
match(v) | on(
  is<int>() >> "int",
  $(is<std::string>()) >> [](const std::string &s) {
    return "str:" + s;
  },
  __ >> [] { return std::string("other"); }
);
```

### `alt<I>()`

Index-based `std::variant` match.

```cpp
match(v) | on(
  alt<0>() >> "first",
  alt<1>() >> "second",
  __ >> "other"
);
```

Rules:

- `is<T>()` requires `T` to appear exactly once.
- `alt<I>()` requires `I` to be in range.
- Use `$()` when you want the alternative value bound into the handler.

---

## Cached Case Packs {#cached-case-packs}

### `static_on(...)`

Cache a stateless `on(...)` factory.

```cpp
match(x) | static_on([] {
  return on(
    lit<1>() >> 1,
    lit<2>() >> 2,
    __ >> 0
  );
});
```

### `PTN_ON(...)`

Convenience macro over `static_on(...)`.

```cpp
match(x) | PTN_ON(
  lit<1>() >> 1,
  lit<2>() >> 2,
  __ >> 0
);
```

The factory must be stateless.

---

## Namespace Summary {#namespace-summary}

The public surface is re-exported through `namespace ptn`:

- `match`
- `on`
- `lit`, `lit_ci`
- `$`, `bind`
- `_`, `__`
- `_0`, `_1`, `_2`, `_3`, `arg`, `rng`
- `has`
- `is`, `alt`

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
    __ >> 0
  );

  return r == 20 ? 0 : 1;
}
```
