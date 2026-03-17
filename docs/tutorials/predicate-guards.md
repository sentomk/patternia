# Custom Predicate Guards

Guards refine an already-bound case.
In Patternia, they live inside the case definition:

```cpp
match(x) | on(
  $[_0 > 0 && _0 < 10] >> "small",
  _ >> "other"
);
```

## Single-Value Guards

Use `_0` for a case that binds exactly one value.

```cpp
match(x) | on(
  $[_0 == 0 || _0 == 1] >> "edge",
  _ >> "other"
);
```

If you want a name instead of `_0`, use `PTN_LET(name, expr)`:

```cpp
match(x) | on(
  $[PTN_LET(value, value == 0 || value == 1)] >> "edge",
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

Use `arg<N>` when a pattern binds multiple values.

```cpp
struct Point {
  int x;
  int y;
};

match(p) | on(
  $(has<&Point::x, &Point::y>)[arg<0> == arg<1>] >> "diagonal",
  _ >> "other"
);
```

If you prefer named guard parameters, use `PTN_WHERE((...), expr)`:

```cpp
match(p) | on(
  $(has<&Point::x, &Point::y>)[PTN_WHERE((x, y), x == y)] >> "diagonal",
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
  $[_0 > 0 && valid_id] >> "valid",
  _ >> "invalid"
);
```

## Guideline

Use placeholder expressions for short, local constraints.
Use lambdas for logic that deserves a name.
