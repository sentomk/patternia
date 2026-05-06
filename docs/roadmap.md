# Roadmap

This page tracks API direction and current implementation status.

Status labels:
- `FINISHED`: already shipped and documented.
- `WIP`: currently being designed or prepared.
- `NEXT`: future consideration, not yet committed.

---

## FINISHED

Completed in [v0.9.3](changelog/v0.9.3.md):

- `neg(p)` negation pattern combinator — inverts sub-pattern match result.
- `val<>` compile-time diagnostic — clear error for runtime-value misuse.
- Variant dispatch threshold lowered from 16 to 8 alternatives.
- Performance Tuning Guide and Common Mistakes Guide.

Completed in [v0.9.2](changelog/v0.9.2.md):

- `any(ps...)`, `all(ps...)` pattern combinators with OR/AND semantics.

Completed in [v0.9.1](changelog/v0.9.1.md):

- `val<V>` static literal entry point.
- C++20 floating-point static literals through `val<V>`.
- Canonical structural entry `has<...>` (factory-style `has<...>()` removed
  from public API).

---

## WIP — v0.10.0

### `match(x) | on(...)` auto-cache

Stateless case packs are automatically memoized into a program-wide static
slot, bringing `on()` to `PTN_ON`-class performance without DSL changes.
128-literal-case dispatch drops from 10.8 ns to 1.13 ns (9.6× speedup).

### Benchmark chart refresh

Replace the current generic chart with targeted comparisons:
- `on()` cached vs uncached vs `PTN_ON` vs hand-written `switch`
- Variant dispatch tiers (hot / warm / cold)
- Packet routing (structural binding + guard overhead)
- Clearer chart styling: direct labels, less noise, readable at a glance.

### Unit test coverage ≥ 95%

Expand test suite to cover edge cases in dispatch, binding, combinators,
and error paths. Target: line coverage ≥ 95% across core headers.

### PTN_ON → deprecated

With auto-cache reaching PTN_ON performance parity, begin the deprecation
cycle. Mark PTN_ON with `[[deprecated]]` attribute pointing users to the
standard `match(x) | on(...)` pipeline.

### New features and API (TBD)

Candidates under evaluation:
- Additional pattern combinators
- Improved binding ergonomics
- Runtime dispatch improvements

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
