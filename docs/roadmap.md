# Roadmap

Planned changes for upcoming releases.
Items listed here are confirmed design decisions, not speculative ideas.

---

## Next Minor Release

### `val<V>` — static literal variable template

Replace `lit<V>()` with `val<V>` as the compile-time literal pattern entry.

`val<V>` is a variable template, consistent with `is<T>` and `alt<I>`.

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

## Future Considerations

Ideas under consideration for later releases. These are not yet confirmed
and may change in scope or form.

### `any(ps...)`, `all(ps...)` — pattern combinators

Combine multiple patterns with or/and semantics in a single case arm.

```cpp
match(x) | on(
  any(val<1>, val<2>, val<3>) >> "1, 2, or 3",
  all(pred(is_positive), pred(is_even)) >> "positive and even",
  _ >> "other"
);
```

Function template form because the arguments are pattern objects, which
may carry runtime state (e.g., `lit(x)`, `pred(f)`).

### `neg(p)` — negation pattern

Invert the result of a sub-pattern.

```cpp
match(x) | on(
  neg(val<0>) >> "non-zero",
  _ >> "zero"
);
```

### `pred(callable)` — predicate pattern

Lift an arbitrary unary predicate into a first-class pattern, evaluated
during the match phase rather than as a post-bind guard.

```cpp
match(x) | on(
  pred([](int v) { return v % 2 == 0; }) >> "even",
  _ >> "odd"
);
```

### `some` / `none` — `std::optional` patterns

Dedicated patterns for the two states of `std::optional`.

```cpp
std::optional<int> v = 42;
match(v) | on(
  $(some) >> [](int x) { return x; },
  none >> 0
);
```

---

## Design Principles for New API

- Stateless compile-time patterns should be variable templates, not
  function templates that return default-constructed objects.
- Function template forms are reserved for patterns that require runtime
  arguments (e.g., `lit(value)`, `lit_ci(value)`, `rng(lo, hi)`).
- Names should be short, lowercase, and read naturally in the DSL.
