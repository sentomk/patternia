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

Use `PTN_BIND` to give multiple bound values readable names.

```cpp
struct Point {
  int x;
  int y;
};

PTN_BIND(Point, x, y);

match(p) | on(
  $(has<&Point::x, &Point::y>)[x == y] >> "diagonal",
  _ >> "other"
);
```

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
