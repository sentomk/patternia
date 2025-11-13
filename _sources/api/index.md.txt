# Patternia API Reference

Patternia is a modern **header-only pattern matching DSL** for expressive and declarative logic in C++17 and above.

This API Reference is generated using **Doxygen + Breathe + MyST**, and reflects
the full public surface of the Patternia library.

---

## Core Layer (`core`)

The **Core Layer** defines the matching engine (`match_builder`),
the DSL bridge (`case_expr`), and the entry-point function `match()`.

```{doxygengroup} core
:project: patternia
:members:
:outline:
```

---

## Pattern Layer (`patterns`)

The **Pattern Layer** provides the built-in patterns shipped with Patternia:

* `value_pattern`
* relational patterns (`lt`, `le`, `gt`, `ge`, `eq`, `ne`)
* `predicate_pattern`
* `and_pattern` / `or_pattern` / `not_pattern`
* `pattern_tag` and `pattern_base`

```{doxygengroup} patterns
:project: patternia
:members:
:outline:
```

---

## Main Public Header

Most users will interact with Patternia via the single entry header:

```{doxygenfile} patternia.hpp
:project: patternia
:members:
:undoc-members:
:outline:
```

> This aggregates the entire DSL, pattern modules, and core engine into a single interface.

---

## Navigation

* **User Guide** — How to write patterns and match expressions
* **Design Notes** — Architecture of Core/Pattern/Type layers
* **Release Notes** — Version history and migration notes

---

Patternia aims to be expressive, extensible, and pleasant to use —
**logic should feel beautiful**.
