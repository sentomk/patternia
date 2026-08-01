# Roadmap

This page tracks API direction and current implementation status.

Status labels:
- `FINISHED`: already shipped and documented.
- `WIP`: currently being designed or prepared.
- `NEXT`: future consideration, not yet committed.

---

## FINISHED

Completed in [v0.9.3](changelog/v0.9.3.md):

- `pred(callable)` predicate pattern for lifting arbitrary unary predicates into first-class patterns.

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

---

### `PTN_BIND` arity 10 — chained arity macros

Raise the named-placeholder limit from 5 to 10 member names and
refactor the arity macros into chained composition
(`PTN_BIND_N` = `PTN_BIND_{N-1}` + one declaration), so future
extensions cost one short macro per level. See PR #44.

---

### Member-anchored `PTN_BIND` placeholders

`PTN_BIND(Type, ...)` names expand to `member_t<&Type::name>`
instead of positional `arg_t<N>`. Guards resolve each name to the
position of its member in the `has<...>` member list at compile
time:

- names follow members, so `has<...>` order no longer matters;
- misspelled member names fail at the `PTN_BIND` line;
- a name used with a `has<...>` that lacks its member fails a
  static_assert.

This also settles the previously deferred `Type`-validation item
without static reflection.

---

### Pattern operator sugar — `!p`, `(a || b)`, `(a && b)`

Operator forms of the combinators: `!p` for `neg(p)`, `(a || b)` for
`any(a, b)`, and `(a && b)` for `all(a, b)`. The overloads live in
`ptn::pat::base` so ADL finds them for every pattern via the shared
`pattern_base` base class, and they reject guard predicates so the
guard-level `&&` / `||` semantics are untouched.

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
