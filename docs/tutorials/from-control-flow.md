# From Control Flow to Pattern Matching

Patternia is useful when branching logic wants to be read as a list of cases
instead of a sequence of checks.

## Value Dispatch

A simple `switch`-style decision maps directly to ordered cases:

```cpp
auto describe(int x) {
  using namespace ptn;

  return match(x) | on(
    lit(0) >> "zero",
    lit(1) >> "one",
    _ >> "many"
  );
}
```

## Constraint-Based Dispatch

Once ordering and overlapping conditions matter, bind the subject explicitly and
attach guards:

```cpp
auto bucket(int x) {
  using namespace ptn;

  return match(x) | on(
    $[_0 < 0] >> 0,
    $[_0 < 10] >> 1,
    $[_0 < 100] >> 2,
    _ >> 3
  );
}
```

## Structured Data

Patternia keeps structure, constraints, and behavior in one visible case list:

```cpp
struct User {
  int age;
  bool active;
};

auto label(const User &u) {
  using namespace ptn;

  return match(u) | on(
    $(has<&User::active>())[arg<0> == false] >> "inactive",
    $(has<&User::age>())[arg<0> < 18] >> "minor",
    _ >> "adult"
  );
}
```

## Takeaway

The shift is simple:

- use literal cases for value dispatch
- use explicit binding and guards for constrained cases
- keep the fallback visible as `_`
