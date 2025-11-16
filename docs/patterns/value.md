# Value Patterns — Architectural Perspective

Value Patterns represent the part of the Pattern System concerned with **runtime values**.  
They describe conditions on concrete data: numbers, strings, states, commands, and any object that can be inspected without involving its type structure.

From an architectural perspective, Value Patterns serve three core roles:

---

## 1. Unifying Value-Level Logic

In conventional C++, comparisons and conditions are expressed through imperative constructs:

- `if (x == 42)`  
- `if (size < capacity)`  
- `if (name == "start")`  
- `if (x > 0 && x < 10)`  

Each condition is tied to surrounding control flow.  
Value Patterns decouple the **expression of conditions** from **the control structure** that uses them.

As a result:

- Value conditions become reusable units.
- Complex value logic becomes declarative—described, not executed.
- Matching rules become centralized and maintainable.

---

## 2. Enabling Compositional Semantics

Patterns obey a formal algebra.  
Value Patterns can be combined with logical operators to describe richer semantics without duplicating comparisons or control logic.

This provides architectural benefits:

- Conditions become **first-class objects**.  
- Rules can be assembled, reused, refined, or shared across modules.  
- Large matching systems can be built from small, orthogonal components.

This capability is essential as the library grows toward supporting predicate guards, custom user-defined patterns, and domain-specific pattern sets.

---

## 3. Integrating with the Match Pipeline

At match time, Value Patterns participate in a common evaluation pipeline:

1. The subject is passed to each pattern.  
2. The pattern decides, independently, whether the subject satisfies its rule.  
3. A matching pattern triggers a handler and short-circuits evaluation.

Value Patterns do not influence the structure of evaluation—only the semantics of the condition.  
This separation of responsibilities keeps the system modular and allows different pattern families to coexist imperatively.

---

## Summary

Architecturally, Value Patterns:

- Represent value-level semantics in declarative form  
- Promote reusable, composable conditions  
- Integrate seamlessly into the shared match pipeline  
- Prepare the ground for higher-level constructs (predicates, guards, domain-specific rules)

They are the foundational layer of runtime data dispatch in Patternia.
