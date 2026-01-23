# From Control Flow to Pattern Matching

## Control Flow as It Grows in Practice

Control flow in C++ is usually learned as a sequence of decisions. You test a condition, you take a branch, and you continue. Over time, this approach becomes second nature: whenever a function needs to “decide,” an `if` or a `switch` appears. The problem is not that these constructs are wrong, but that they silently take on too many responsibilities as code evolves.

A simple decision today often becomes a policy tomorrow. Conditions start to overlap. Temporary variables appear only to shuttle values between checks. Eventually, the structure of the logic is no longer visible from the code itself. Pattern matching in Patternia is designed to address this drift by changing how decisions are expressed, without requiring a different language or runtime model.

---

## Simple Decisions Deserve Simple Matching

The first idea to internalize is that **not all branching deserves the same level of expressiveness**. Patternia reflects this by offering a deliberately simple entry point and a more expressive form, instead of one universal mechanism that tries to do everything.

When a decision is purely about *which value you have*, Patternia expects you to say exactly that—no more, no less. This is the role of `match(x, cases())`.

```cpp
auto describe(int x) {
  return match(x, cases(
    lit(0) >> "zero",
    lit(1) >> "one",
    __     >> "many"
  )).end();
}
```

This form is intentionally constrained. Each case is a direct mapping from a pattern to a result. There is no binding, no guard, and no hidden control flow. The reader can understand the entire decision at a glance. This is not a limitation to be worked around; it is a design boundary. As long as your logic fits naturally into a `switch`, `cases()` is where it belongs.

---

## When Value Dispatch Is No Longer Enough

The moment that boundary is crossed, imperative code usually starts to stretch. Consider logic that depends not just on a value, but on a property of that value:

```cpp
int bucket(int x) {
  if (x < 0) return 0;
  if (x < 10) return 1;
  if (x < 100) return 2;
  return 3;
}
```

This is no longer value dispatch. The conditions overlap, and the ordering matters. Trying to force this logic into a value-based case list would hide the intent rather than clarify it. Patternia does not ask you to bend `cases()` to accommodate this. Instead, it asks you to change how the decision itself is modeled.

---

## Making Data Flow Explicit with the Builder Form

At this point, you move to the builder form, `match(x).when(...)`, and you make data flow explicit through binding:

```cpp
auto bucket(int x) {
  return match(x)
    .when(bind()[_ < 0]   >> 0)
    .when(bind()[_ < 10]  >> 1)
    .when(bind()[_ < 100] >> 2)
    .otherwise(3);
}
```

What matters here is not the syntax, but the semantics. Each case now describes a complete rule: bind the value, require a condition to hold, then produce a result. Guards are not arbitrary filters; they are part of the case definition itself. If a guard fails, the case simply does not exist for that input.

This distinction—between *selecting a case* and *refining a case*—is fundamental. Guards are not available in `cases()` precisely because `cases()` is not about refinement. It is about enumeration. Once you need refinement, Patternia requires you to make that step visible in the structure of the code.

---

## Separating Structure, Constraints, and Behavior

The same principle applies when decisions depend on structured data. In imperative code, it is common to see fields accessed and validated inline, often with early returns. Over time, this produces handlers that are half validation and half behavior.

Patternia encourages a different organization. Structure is matched first, data is bound explicitly, and semantic constraints are expressed as guards. The handler then operates under the assumption that all prerequisites are satisfied.

```cpp
struct User {
  int age;
  bool active;
};

auto label(const User& u) {
  return match(u)
    .when(
      bind(has<&User::active>())[arg<0> == false] >>
      "inactive"
    )
    .when(
      bind(has<&User::age>())[arg<0> < 18] >>
      "minor"
    )
    .otherwise("adult");
}
```

Notice what does *not* happen in this code. The handler does not inspect fields. It does not re-check conditions. All of that logic lives on the boundary between cases. This is where pattern matching begins to pay off: behavior becomes simpler because eligibility is enforced declaratively.

---

## Fallbacks as Explicit Design Choices

Another important aspect of Patternia’s design is how it treats fallback behavior. In traditional control flow, `default` branches often serve multiple purposes: they catch errors, handle normal fallthrough, or act as a safety net. Patternia separates these roles.

A wildcard pattern (`__`) is an explicit, ordered case. A match-level fallback (`otherwise`) is only invoked when no case applies at all. This separation prevents accidental behavior changes when new cases are added and makes the intent of fallback logic explicit.

---

## Binding and Guards as a Discipline, Not a Convenience

Throughout this progression, one rule remains consistent: **binding is always explicit, and guards only exist after binding**. You never “half-bind” a value, and you never attach a guard to a case that does not declare what data it operates on. This makes the relationship between a case and its handler mechanically obvious. The handler’s parameters are not inferred from context; they are earned by binding.

This discipline is what allows Patternia code to remain readable even as it grows. Each increase in expressiveness is accompanied by a visible structural change in the code.

---

## Thinking in Cases Instead of Branches

Seen as a whole, Patternia does not replace `if` or `switch`. It reframes them. Simple decisions stay simple through `match(x, cases())`. Rich decisions become structured through `.when(...)`, `bind()`, and guards. Instead of growing a single control-flow construct until it collapses under its own weight, you move to a different form when the problem itself changes shape.

That shift—from procedural branching to declarative case description—is what “getting into pattern matching” ultimately means in Patternia. It is not about learning new syntax. It is about learning when to stop asking *“how do I check this?”* and start stating *“these are the cases that exist.”*
