# Roadmap

This page tracks API direction and current implementation status.

Status labels:
- `FINISHED`: already shipped and documented.
- `WIP`: currently being designed or prepared.
- `NEXT`: future consideration, not yet committed.

---

## FINISHED

Completed in [v0.9.4](changelog/v0.9.4.md):

- `PTN_BIND(Type, names...)` member-anchored named guard
  placeholders (one to ten names), replacing the positional guard
  APIs (`__`, `_0`, `arg<N>`, `PTN_LET`, `PTN_WHERE`).
- Block-scope `PTN_BIND` names usable inside `PTN_ON`.
- Pattern operator sugar: `!p`, `(a || b)`, `(a && b)`.

Completed in [v0.9.3](changelog/v0.9.3.md):

- `pred(callable)` predicate pattern for lifting arbitrary unary predicates into first-class patterns.
- `neg(p)` negation pattern.

Completed in [v0.9.2](changelog/v0.9.2.md):

- `any(ps...)`, `all(ps...)` pattern combinators with OR/AND semantics.

Completed in [v0.9.1](changelog/v0.9.1.md):

- `val<V>` static literal entry point.
- C++20 floating-point static literals through `val<V>`.
- Canonical structural entry `has<...>` (factory-style `has<...>()` removed
  from public API).

---

## WIP

No active workstream. See NEXT for candidates.

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

### Multi-subject matching — `match(a, b)` with slot composition

Match on combinations of values, in the spirit of Rust's
`match (a, b)`. `match(a, b)` packs the subjects into a tuple; a
slot-wise pattern (working name `tup(...)`, alternative `each(...)`)
matches each position with its own sub-pattern and flattens the
bindings in order:

```cpp
match(x, y) | on(
  tup(lit(0), lit(0)) >> "origin",
  tup(lit(0), _)      >> "on y axis",
  tup(_, lit(0))      >> "on x axis",
  _                   >> "elsewhere"
);
```

Design notes:

- Pure front-end sugar: the engine is untouched; `tup` is a pattern
  combinator in the same family as `any`/`all`.
- Shares the slot-composition machinery (per-slot match/bind plus
  binding flattening) with `some`/`none`. The two should be designed
  together so their binding semantics stay consistent.

---

## Design Principles for New API

- Stateless compile-time patterns should be variable templates, not
  function templates that return default-constructed objects.
- Function template forms are reserved for patterns that require runtime
  arguments (e.g., `lit(value)`, `lit_ci(value)`, `rng(lo, hi)`).
- Names should be short, lowercase, and read naturally in the DSL.

## Considered and Declined

Syntax ideas that were evaluated and intentionally rejected, kept
here so they are not re-proposed without new motivation.

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
  happens at declaration time (see WIP above).
