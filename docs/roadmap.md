# Roadmap

This page tracks API direction and current implementation status.

Status labels:
- `FINISHED`: already shipped and documented.
- `WIP`: currently being designed or prepared.
- `NEXT`: future consideration, not yet committed.

---

## FINISHED

Completed in [v0.9.2](changelog/v0.9.2.md):

- `any(ps...)`, `all(ps...)` pattern combinators with OR/AND semantics.

Completed in [v0.9.1](changelog/v0.9.1.md):

- `val<V>` static literal entry point.
- C++20 floating-point static literals through `val<V>`.
- Canonical structural entry `has<...>` (factory-style `has<...>()` removed
  from public API).

---

## WIP

Current workstream for upcoming releases.

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

---

## NEXT

Potential follow-up items after current WIP scope is stabilized.

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
