# Roadmap

Planned changes for upcoming releases.
Items listed here are confirmed design decisions, not speculative ideas.

---

## Next Minor Release

### `val<V>` — static literal variable template

Replace `lit<V>()` with `val<V>` as the compile-time literal pattern entry.

`val<V>` is a variable template, consistent with `is<T>`, `alt<I>`, and
the planned `has<Ms...>` change below.

```cpp
// current
match(x) | PTN_ON(
  lit<1>() >> 1,
  lit<2>() >> 2,
  _ >> 0
);

// planned
match(x) | PTN_ON(
  val<1> >> 1,
  val<2> >> 2,
  _ >> 0
);
```

`lit(value)`, `lit_ci(value)` remain unchanged as runtime forms.

### `has<Ms...>` — structural pattern variable template

Replace `has<Ms...>()` (function template) with `has<Ms...>` (variable
template). No functional change; removes the trailing `()`.

```cpp
// current
$(has<&Point::x, &Point::y>()) >> handler

// planned
$(has<&Point::x, &Point::y>) >> handler
```

### C++20 floating-point `val<V>` support

When compiled in C++20 or later mode, `val<V>` will accept floating-point
non-type template parameters, enabling static dispatch for floating-point
literals.

```cpp
// C++20 only
match(x) | on(
  val<3.14> >> "pi",
  _ >> "other"
);
```

C++17 builds are unaffected. Floating-point matching continues to use
`lit(3.14)`.

---

## Design Principles for New API

- Stateless compile-time patterns should be variable templates, not
  function templates that return default-constructed objects.
- Function template forms are reserved for patterns that require runtime
  arguments (e.g., `lit(value)`, `lit_ci(value)`, `rng(lo, hi)`).
- Names should be short, lowercase, and read naturally in the DSL.
