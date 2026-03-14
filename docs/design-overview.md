# Design Overview

Patternia treats pattern matching as an expression over a concrete subject:

```cpp
match(subject) | on(
  case_1,
  case_2,
  _ >> fallback
)
```

The library is built around four ideas:

1. Subject flow is explicit.
2. Binding is explicit.
3. Cases are ordered and first-match-wins.
4. The public syntax stays small.

---

## Subject-First Matching

Patternia does not hide the matched value inside an implicit control-flow form.
You always start with the subject:

```cpp
match(x) | on(
  lit(1) >> "one",
  _ >> "other"
);
```

This makes the match expression read as data flow rather than as a block-level
statement form.

---

## Immediate Evaluation

`match(subject)` by itself is only the left side of the pipeline.
Evaluation happens when the `on(...)` case pack is supplied:

```cpp
auto result = match(x) | on(
  lit(0) >> 0,
  lit(1) >> 1,
  _ >> -1
);
```

There is no separate chained builder API.

---

## Explicit Binding

Matching and binding are deliberately separate.
Patterns answer "does this match?"
Binding answers "what values are exposed to the handler?"

```cpp
match(x) | on(
  $ >> [](int v) { return v; },
  _ >> 0
);
```

```cpp
match(p) | on(
  $(has<&Point::x, &Point::y>()) >> [](int x, int y) {
    return x + y;
  },
  _ >> 0
);
```

This separation keeps handler signatures predictable and keeps data flow local.

---

## Guards Refine a Bound Case

Guards apply after a binding pattern succeeds.

```cpp
match(x) | on(
  $[PTN_LET(value, value > 0 && value < 10)] >> "small",
  _ >> "other"
);
```

The evaluation model is:

1. Match the pattern.
2. Bind values.
3. Evaluate the guard.
4. Run the handler on success.

If the guard fails, matching continues with the next case.

---

## Wildcard Fallback

Patternia requires an explicit fallback case in `on(...)`.

```cpp
match(v) | on(
  is<int> >> "int",
  is<std::string> >> "string",
  _ >> "other"
);
```

This keeps the terminal behavior visible inside the case list itself.

---

## Structural and Variant Matching Share One Core

Literal matching, structural matching, guarded binding, and variant dispatch all
lower through the same ordered case model.

```cpp
match(value) | on(
  lit(1) >> "literal",
  $(has<&Point::x, &Point::y>()) >> [](int x, int y) {
    return x + y > 0 ? "point" : "origin-side";
  },
  $(is<std::string>)[PTN_LET(text, text != "")] >> "string",
  _ >> "other"
);
```

That shared model is what keeps the DSL small without giving up expressive
power.

---

## Performance Direction

Patternia aims at zero-overhead abstractions:

- no virtual dispatch
- no RTTI
- no heap allocation
- aggressive inlining and compile-time validation

For repeated hot paths, users can cache case packs with `static_on(...)` or
`PTN_ON(...)`.

---

## Summary

Patternia is intentionally narrow:

- one subject-first entry
- one case-pack terminal form
- explicit binding
- explicit wildcard fallback

That constraint is part of the design.
