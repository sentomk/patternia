# Custom Predicate Guards

Guards refine an already-bound case.
In Patternia, they live inside the case definition:

```cpp
match(x) | on(
  $[_ > 0 && _ < 10] >> "small",
  _ >> "other"
);
```

## Single-Value Guards

Use `_` for a case that binds exactly one value.

```cpp
match(x) | on(
  $[_ == 0 || _ == 1] >> "edge",
  _ >> "other"
);
```

For a single bound value, `_` is the guard placeholder:

```cpp
match(x) | on(
  $[_ == 0 || _ == 1] >> "edge",
  _ >> "other"
);
```

Use `rng(...)` when a range reads better than two comparisons:

```cpp
match(x) | on(
  $[rng(0, 10)] >> "closed",
  $[rng(0, 10, pat::mod::open)] >> "open",
  _ >> "other"
);
```

## Multi-Value Guards

Use a C++17 structured binding to give structural members readable placeholder
names. These names refer to members, not extracted values:

```cpp
struct Point {
  int x;
  int y;
};

constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;

match(p) | on(
  $(pt)[x == y] >> "diagonal",
  _ >> "other"
);
```

The names follow the members listed in `has<...>`; `_ign` slots are skipped.
`$(pt)` remains responsible for binding those members to the handler.

`PTN_BIND` is deprecated but still supported. Automatic structured bindings
inside a function work with `on(...)`, as above, but cannot be used inside
`PTN_ON`'s captureless lambda. To cache that case pack, declare the pattern
and names at namespace scope. In C++20, function-local declarations can use
`static constexpr auto pt = has<&Point::x, &Point::y>;` and
`static auto [x, y] = pt;`. In C++17, keep `PTN_BIND` for local names and give
any local pattern used by `PTN_ON` static storage too.

## Use Lambdas for Domain Logic

When the rule is not a short relational expression, use a predicate.

```cpp
auto is_prime = [](int v) {
  if (v < 2) {
    return false;
  }
  for (int i = 2; i * i <= v; ++i) {
    if (v % i == 0) {
      return false;
    }
  }
  return true;
};

match(x) | on(
  $[is_prime] >> "prime",
  _ >> "composite"
);
```

You can mix a placeholder expression with a named predicate:

```cpp
auto valid_id = [](int v) {
  return v % 7 == 0;
};

match(x) | on(
  $[_ > 0 && valid_id] >> "valid",
  _ >> "invalid"
);
```

## Guideline

Use placeholder expressions for short, local constraints.
Use lambdas for logic that deserves a name.
