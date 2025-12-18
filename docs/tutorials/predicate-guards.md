# Custom Predicate Guards

## Guards as Part of Case Definition

In Patternia, a guard is not a free-floating condition. It is not an `if` placed near a handler, nor a generic predicate applied to the input value. A guard belongs to a *case*, and more specifically, it belongs to a case that has already bound data.

This is why guards only appear in the `when(...)` form, and only after a pattern introduces bindings. A guard refines an already matched and bound case. If the guard fails, the case simply does not apply, and matching continues. Nothing “falls through,” and nothing partially executes.

Understanding this placement is the key to using guards correctly. Once that boundary is clear, the remaining question becomes practical: **what kind of logic should live inside a guard?**

---

## Using Built-in Guard Expressions for Simple Logic

Patternia provides a small set of operator overloads designed specifically for guard expressions. These operators allow you to express simple, local constraints directly, without dropping back into lambdas.

For a single bound value, this often reads naturally:

```cpp
match(x)
  .when(bind()[_ > 0 && _ < 10] >> "small")
  .otherwise("other");
```

Here, `_` is not a runtime variable. It is a placeholder used to construct a predicate over the bound value. The expression `_ > 0 && _ < 10` does not evaluate immediately; it builds a guard that will be evaluated after binding.

This style is intended for *simple, relational constraints*: comparisons, ranges, and boolean combinations. The goal is clarity, not expressiveness at all costs. If the condition can be read as a short sentence about the value, it likely belongs in this form.

Logical composition using `&&` and `||` is fully supported and is often sufficient for straightforward rules:

```cpp
.when(bind()[_ == 0 || _ == 1] >> "edge")
```

As long as the logic remains local and obvious, keeping it in expression form improves readability by keeping the rule close to the case definition.

---

## Multi-Value Guards and Explicit Relationships

When a pattern binds multiple values, guards can express relationships between them. In this situation, the placeholder `_` is no longer sufficient, because there is more than one bound value. Patternia makes this explicit by requiring indexed access via `arg<N>`.

```cpp
.when(
  bind(has<&Point::x, &Point::y>())
    [arg<0> == arg<1>] >>
  "diagonal"
)
```

Here, `arg<0>` and `arg<1>` refer to the first and second bound values, in binding order. This makes the dependency between values explicit and avoids ambiguity. The guard describes a relationship between bindings, not an opaque condition over the original object.

As with single-value guards, logical operators can be used to combine constraints, but the same rule applies: if the condition reads cleanly as a declarative relationship, it fits well in this form.

---

## Knowing When to Stop Using Expression Guards

Not all logic belongs in an expression-based guard. Patternia deliberately does **not** try to turn guards into a full embedded language. Once a condition becomes algorithmic, stateful, or domain-specific, expression syntax stops helping.

Consider a check such as “is this number prime?” or “does this payload satisfy protocol invariants?” Attempting to encode such logic through operator overloads would obscure intent rather than clarify it.

In these cases, the correct approach is to step back into ordinary C++ and use a lambda:

```cpp
auto is_prime = [](int v) {
  if (v < 2) return false;
  for (int i = 2; i * i <= v; ++i)
    if (v % i == 0) return false;
  return true;
};

match(x)
  .when(bind()[is_prime] >> "prime")
  .otherwise("composite");
```

This is not a fallback or a workaround; it is the intended design. Guards are predicates. When a predicate is complex, it should look like a predicate, written in normal C++.

The lambda form keeps complex logic readable, testable, and reusable, without forcing it into an artificial DSL.

---

## Combining Expression Guards and Lambdas

Expression guards and lambda guards are not mutually exclusive. They can be mixed naturally, as long as the rule remains clear.

```cpp
auto valid_id = [](int v) {
  return v % 7 == 0;
};

match(x)
  .when(bind()[_ > 0 && valid_id] >> "valid")
  .otherwise("invalid");
```

In this example, the simple relational constraint (`_ > 0`) stays in expression form, while the domain-specific rule (`valid_id`) is expressed as a lambda. The guard reads as a composition of two ideas, each expressed in its most natural form.

This composition works because guards are predicates, not control flow. Whether a predicate comes from an expression or a callable does not change how the case behaves.

---

## Guard Logic Belongs at the Boundary

A recurring theme in Patternia is that **validation belongs at the boundary, not inside the handler**. Guards exist to enforce that rule. A handler should assume its preconditions are satisfied and focus solely on behavior.

If you find yourself writing defensive checks inside a handler, it is often a sign that the logic belongs in a guard instead. Conversely, if a guard starts to resemble a small program, it likely belongs in a lambda or outside the match entirely.

Patternia does not attempt to blur this line. It gives you just enough syntax to express common constraints declaratively, and then relies on standard C++ for everything else.

---

## Writing Guards with Intent

Effective guard usage is less about knowing every operator and more about restraint. Use the built-in expression operators for small, obvious constraints. Use `&&` and `||` to combine them when the relationship remains readable. When logic becomes complex, name it and write it as a lambda.

By following this discipline, guards remain what they are meant to be: precise refinements of cases, not miniature control-flow systems hidden inside brackets.

This is the role of custom predicate guards in Patternia—not to replace C++, but to clarify where conditions belong.
