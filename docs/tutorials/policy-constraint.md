# Policy Constraint Matching

## From Ad-Hoc Rules to Declarative Policies

Policy-based decisions are a common source of complexity in real-world systems. Access control, feature gating, pricing rules, and workflow authorization often begin as a handful of conditions, then gradually accumulate exceptions, overrides, and special cases.

In traditional C++, this logic is usually expressed as a growing chain of `if` statements. Each new rule is “inserted” somewhere into the control flow, and the meaning of the code becomes inseparable from its ordering. The logic works, but understanding *why* a decision was made requires tracing execution step by step.

Patternia approaches policy decisions differently. Instead of encoding policy as control flow, it treats each rule as a *case* defined by structure and constraints. The goal is not to evaluate conditions procedurally, but to describe which policies exist and when they apply.

---

## Defining the Policy Domain

We start by modeling the data involved in the decision. A policy decision typically depends on who is making the request, what is being accessed, and how it is accessed.

```cpp
enum class Role {
  Guest,
  User,
  Admin
};

struct Request {
  Role role;
  int  resource_id;
  bool read_only;
};
```

This structure already hints at the nature of the problem. There is no single field that determines the outcome. The decision depends on relationships between fields, which immediately places us outside the realm of value dispatch.

---

## Recognizing a Constraint-Based Match

Because the outcome cannot be determined by enumerating values, `cases()` is not the right tool here. What we need is a set of rules, each defined by constraints over the request data. This is exactly the scenario where `when(...)` and guarded bindings are intended to be used.

```cpp
match(req)
  .when(/* policy rule */)
  .when(/* policy rule */)
  .otherwise(/* default policy */);
```

At this level, the match already reads as a policy table rather than as a branching program.

---

## Binding the Relevant Dimensions of a Rule

Each policy rule depends on a specific subset of the request fields. Instead of passing the entire `Request` object into every handler, we bind only the data that the rule actually reasons about.

```cpp
bind(has<&Request::role, &Request::resource_id, &Request::read_only>())
```

This binding step defines the vocabulary of the rule. Everything that follows—the guard and the handler—will operate strictly in terms of these bound values. The rule does not “peek” back into the original object.

---

## Expressing Unconditional Policies Declaratively

Some policies are unconditional within their scope. For example, administrators may be allowed to access any resource regardless of mode.

```cpp
.when(
  bind(has<&Request::role>())
    [arg<0> == Role::Admin] >>
  true
)
```

This case reads as a direct statement of policy. There is no procedural logic hidden in the handler. The policy itself is expressed entirely in the pattern and guard.

---

## Combining Multiple Constraints in a Single Rule

More realistic policies involve multiple conditions. Consider a rule that allows regular users to access resources only in read-only mode, and only when the resource identifier is valid.

```cpp
.when(
  bind(has<&Request::role, &Request::resource_id, &Request::read_only>())
    [
      arg<0> == Role::User &&
      arg<1> >= 0 &&
      arg<2> == true
    ] >>
  true
)
```

This is a **multi-value guard**, and therefore `_` does not exist here. Every relationship is spelled out explicitly in terms of bound values. The guard describes *why* the rule applies; the handler simply enacts the decision.

---

## Introducing Domain-Specific Predicates

Some policy constraints cannot be expressed as simple relational checks. Whether a resource is public, restricted, or archived may depend on external logic or domain knowledge. Attempting to encode this logic directly into expression guards would reduce clarity.

Instead, we name the predicate:

```cpp
auto is_public_resource = [](int id) {
  return id < 1000;
};
```

And then integrate it into the rule:

```cpp
.when(
  bind(has<&Request::role, &Request::resource_id>())
    [
      arg<0> == Role::Guest &&
      is_public_resource
    ] >>
  true
)
```

This mirrors the approach taken in geometric constraints: arithmetic-heavy logic lives in a predicate; simple relational logic remains declarative.

---

## Declaring the Default Policy Explicitly

Policy systems must always answer the question “what happens if no rule applies?”. Patternia forces this answer to be explicit.

```cpp
.otherwise(false);
```

This is not a fallback rule among others; it is a statement of intent. Requests that do not satisfy any declared policy are denied by default.

---

## Reading Policies as Patterns, Not Branches

Putting the rules together yields a complete policy matcher:

```cpp
bool allowed(const Request& req) {
  return match(req)

    .when(
      bind(has<&Request::role>())
        [arg<0> == Role::Admin] >>
      true
    )

    .when(
      bind(has<&Request::role, &Request::resource_id, &Request::read_only>())
        [
          arg<0> == Role::User &&
          arg<1> >= 0 &&
          arg<2> == true
        ] >>
      true
    )

    .when(
      bind(has<&Request::role, &Request::resource_id>())
        [
          arg<0> == Role::Guest &&
          is_public_resource
        ] >>
      true
    )

    .otherwise(false);
}
```

Each policy is a self-contained case. The order is visible. The constraints are explicit. The handler code is trivial because all reasoning has already been done.

---

## Policies as Declarative Constraints

The important takeaway from this example is not how access control works, but how constraints scale when treated as patterns. Each policy is a description of *when it applies*, not a fragment of control flow.

This mirrors the lesson from geometric constraint matching. Whether the constraint is mathematical or organizational, the structure is the same: bind what matters, declare why the case applies, and keep behavior separate from validation.

This is the essence of constraint-based matching in Patternia.
