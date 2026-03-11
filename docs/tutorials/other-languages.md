# Pattern Matching in Other Languages

If you already know Rust, Scala, Haskell, or OCaml, Patternia should feel
familiar at the semantic level:

- matching is expression-oriented
- cases are ordered
- the first matching case wins
- fallback is explicit

## Expression-Oriented Matching

Rust:

```rust
let label = match x {
    0 => "zero",
    1 => "one",
    _ => "many",
};
```

Patternia:

```cpp
auto label = match(x) | on(
  lit(0) >> "zero",
  lit(1) >> "one",
  __ >> "many"
);
```

## Explicit Binding

Many languages bind inside the pattern syntax.
Patternia keeps binding explicit because it is a C++ library:

```cpp
match(value) | on(
  bind() >> [](auto v) { process(v); },
  __ >> [] {}
);
```

## Guards

Patternia guards refine a bound case:

```cpp
auto label = match(x) | on(
  bind()[_0 > 0 && _0 < 10] >> "small",
  __ >> "other"
);
```

## Structural Matching

Without native language destructuring, Patternia uses explicit member pointers:

```cpp
auto label = match(user) | on(
  bind(has<&User::age, &User::active>())[arg<0> < 18 && arg<1> == true]
      >> "minor",
  __ >> "adult"
);
```

## Mental Model

Patternia does not try to copy another language's syntax.
It translates the same ideas into a small C++ DSL:

- explicit subject
- explicit bindings
- explicit fallback
