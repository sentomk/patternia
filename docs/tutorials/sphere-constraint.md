# Geometric Constraint Matching

Geometric rules are often clearer as named constraints than as inline `if`
statements.

```cpp
struct Vec3 {
  double x;
  double y;
  double z;
};
```

## Name the Predicate

```cpp
auto inside_unit_sphere = [](double x, double y, double z) {
  return x * x + y * y + z * z < 1.0;
};
```

## Use It as a Guard

```cpp
const char *classify(const Vec3 &p) {
  using namespace ptn;

  return match(p) | on(
    $(has<&Vec3::x, &Vec3::y, &Vec3::z>())[inside_unit_sphere] >> "inside",
    _ >> "outside"
  );
}
```

## Refining the Constraint

You can combine a named predicate with `arg<N>` for simple extra checks:

```cpp
const char *classify_upper(const Vec3 &p) {
  using namespace ptn;

  return match(p) | on(
    $(has<&Vec3::x, &Vec3::y, &Vec3::z>())[arg<2> >= 0 && inside_unit_sphere]
        >> "inside",
    _ >> "outside"
  );
}
```

For multi-value guards, prefer explicit `arg<N>` references or a callable that
accepts all bound values.
