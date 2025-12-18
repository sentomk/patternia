# Geometric Constraint Matching

## From Numeric Conditions to Declarative Shape Constraints

Geometric constraints are a good stress test for any control-flow abstraction. They are simple to state mathematically, yet often awkward to express clearly in code. A classic example is the unit sphere: a point lies inside the sphere if and only if
`x² + y² + z² < 1`.

In traditional C++, this condition is usually written as a boolean expression embedded directly inside an `if` statement. The code is compact, but the intent tends to blur once additional decisions are layered on top.

```cpp
bool inside_unit_sphere(const Vec3& p) {
  return p.x * p.x + p.y * p.y + p.z * p.z < 1.0;
}
```

Patternia approaches the same problem from a different angle. Instead of asking “is this condition true?”, it asks “does this value belong to a case defined by this constraint?”. That shift becomes more visible as the example grows.

---

## Choosing the Subject of the Match

The subject of the decision is the point itself:

```cpp
struct Vec3 {
  double x;
  double y;
  double z;
};
```

There is no finite set of values to enumerate, so this is not a `cases()` problem. We are dealing with constraints, not value dispatch, which means we move directly to the `when(...)` form.

---

## Binding Structure Before Reasoning About Constraints

The geometric constraint depends on the coordinates of the point. Rather than accessing those fields inside a handler, Patternia encourages binding them explicitly as part of the case definition.

```cpp
match(p)
  .when(
    bind(has<&Vec3::x, &Vec3::y, &Vec3::z>())
      /* guard */
      >>
    /* handler */
  )
  .otherwise(/* fallback */);
```

At this stage, nothing has been “checked” yet. The code simply states that this case is about points for which `x`, `y`, and `z` are relevant. Binding defines *what data the case operates on* before any constraint is applied.

---

## Writing the Constraint as a Predicate

The constraint `x² + y² + z² < 1` is not a simple relational check. It involves arithmetic across multiple values. This is exactly the point where Patternia expects you to step out of expression-based guards and write a predicate.

```cpp
auto inside_unit_sphere = [](double x, double y, double z) {
  return x * x + y * y + z * z < 1.0;
};
```

This predicate is ordinary C++. It can be tested independently and reused elsewhere. Patternia does not attempt to encode this kind of logic into operator overloads, because doing so would obscure intent rather than clarify it.

---

## Attaching a Multi-Value Guard Correctly

With multiple bound values, the guard operates over **all bindings at once**. In this context, it is important to be precise:

**There is no `_` in multi-value guards.**

The placeholder `_` exists only for single-value bindings. Once a pattern binds more than one value, guards must refer to bound values explicitly—either via `arg<N>` or by using a callable that consumes all bound values.

In this case, the lambda form is the most natural:

```cpp
auto classify(const Vec3& p) {
  return match(p)
    .when(
      bind(has<&Vec3::x, &Vec3::y, &Vec3::z>())
        [inside_unit_sphere] >>
      "inside"
    )
    .otherwise("outside");
}
```

The guard is evaluated only after binding succeeds. If the predicate returns false, the case does not apply and matching continues. The handler itself contains no geometric logic; it simply represents the outcome of a satisfied constraint.

---

## Adding Additional Constraints Without Confusing Semantics

Suppose we want to refine the rule: the point must be inside the unit sphere *and* lie in the upper half-space (`z ≥ 0`). This introduces a simple relational constraint alongside the existing geometric one.

Because this is a **multi-value guard**, `_` is still not available. We must express the additional constraint explicitly in terms of bound values.

One option is to extend the predicate:

```cpp
auto inside_upper_unit_sphere = [](double x, double y, double z) {
  return z >= 0 && x * x + y * y + z * z < 1.0;
};
```

Another option is to combine predicates using `arg<N>` for the simple part:

```cpp
auto classify(const Vec3& p) {
  return match(p)
    .when(
      bind(has<&Vec3::x, &Vec3::y, &Vec3::z>())
        [arg<2> >= 0 && inside_unit_sphere] >>
      "inside"
    )
    .otherwise("outside");
}
```

Here, `arg<2>` refers to the third bound value (`z`), following binding order. The guard reads as a clear composition: a simple relational check combined with a domain-specific predicate.

The important rule remains intact: **multi-value guards are explicit**. Nothing is implicit, and no placeholder stands in for “the value” when there are multiple values involved.

---

## Why This Structure Matters

At this point, the benefit over a raw `if` statement becomes clear. The match expression does not merely compute a boolean; it describes a *case* defined by a geometric constraint. Binding declares which data the case depends on. The guard declares why the case applies. The handler declares what happens when it does.

As constraints grow—more dimensions, more regions, more classifications—the match grows by adding or refining cases, not by nesting conditionals. This is the same scaling property that makes pattern matching attractive in other languages, applied here to geometric reasoning.

---

## Constraints as Patterns, Not Conditions

The lesson of this example is not how to encode `x² + y² + z² < 1`. It is how to recognize constraints as patterns. Once a constraint is treated as a case boundary rather than an inline check, it becomes composable, testable, and readable.

Patternia’s rules around guards—especially the strict separation between single-value placeholders and multi-value bindings—exist to preserve that clarity. They prevent ambiguity and make the structure of a decision visible in the code itself.

This is the purpose of worked examples in Patternia: to show how simple rules, applied consistently, scale to real problems without collapsing back into ad-hoc control flow.
