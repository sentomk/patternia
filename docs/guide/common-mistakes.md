## Common Mistakes

This guide covers typical errors when using Patternia and explains what the compiler diagnostics mean.

## Passing runtime value to val<>

The `val<V>` pattern is for compile-time constants. It allows the compiler to generate optimized jump tables or direct comparisons.

```cpp
int main(int argc, char** argv) {
  int target = std::atoi(argv[1]);
  int x = 42;

  // Error: target is not a constant expression
  auto result = match(x) | on(
    val<target> >> true,
    _ >> false
  );
}
```

> What the compiler says:
> "non-type template argument is not a constant expression" or "the value of 'target' is not usable in a constant expression".

**Fix:** Use `lit(v)` for runtime values.

```cpp
auto result = match(x) | on(
  lit(target) >> true,
  _ >> false
);
```

## Missing wildcard fallback

Patternia requires exhaustive matching. If you don't provide a catch-all case, the matcher won't compile because it cannot guarantee a return value for all possible inputs.

```cpp
int describe(int x) {
  return match(x) | on(
    lit(0) >> 0,
    lit(1) >> 1
    // Error: no fallback provided
  );
}
```

> What the compiler says:
> "static_assert failed: match must be exhaustive" or a long error involving `unresolved_match` types.

**Fix:** Add a wildcard `_` fallback.

```cpp
return match(x) | on(
  lit(0) >> 0,
  lit(1) >> 1,
  _ >> -1
);
```

## Lambda captures in PTN_ON

The `PTN_ON` macro caches the entire matcher in a function-local static variable. This means any handlers inside it must be stateless.

```cpp
int search(int x, int limit) {
  return match(x) | PTN_ON(
    $[PTN_LET(v, v < limit)] >> [] { return true; }, // Error: 'limit' cannot be captured
    _ >> [] { return false; }
  );
}
```

> What the compiler says:
> "a static variable cannot have a non-static data member as a capture" or "lambda in a static context cannot capture variables".

**Fix:** Use the raw `on(...)` pipeline if you need captures, or pass state through a handler struct.

```cpp
return match(x) | on(
  $[PTN_LET(v, v < limit)] >> [] { return true; },
  _ >> [] { return false; }
);
```

## Misusing the binding wildcard `$` alone vs `$(pattern)`

The `$` wildcard binds the entire subject to a handler argument. If you want to bind a specific sub-pattern or type, you must wrap it in `$(...)`.

```cpp
using Value = std::variant<int, std::string>;

void process(Value v) {
  match(v) | on(
    // Wrong: $ alone matches everything and binds it as Value
    $ >> [](int x) { /* ... */ }, 
    
    // Correct: $(is<int>) binds only when it's an int
    $(is<int>) >> [](int x) { /* ... */ },
    _ >> []{}
  );
}
```

> What the compiler says:
> "no matching function for call to object of type '(lambda)'" because the handler expects `int` but `$` provides the original subject type.

**Fix:** Use `$(pattern)` to bind specific patterns. Use `$` only when you want to bind the whole subject regardless of its internal structure.

## Handler arity mismatch

The number of arguments in your lambda handler must exactly match the number of bindings produced by the pattern.

```cpp
struct Point { int x; int y; };

void move(Point p) {
  match(p) | on(
    $(has<&Point::x, &Point::y>) >> [](int x) { // Error: pattern binds 2 values, handler takes 1
      std::cout << x << "\n";
    },
    _ >> []{}
  );
}
```

> What the compiler says:
> "static_assert failed: handler arity does not match pattern bindings" or "too few arguments to function call".

**Fix:** Match the handler parameters to the pattern bindings.

```cpp
$(has<&Point::x, &Point::y>) >> [](int x, int y) {
  std::cout << x << ", " << y << "\n";
}
```

## Return type inconsistency

When using `match` as an expression, every case must return the same type (or types that share a common result type like a base class or `std::common_type`).

```cpp
auto result = match(x) | on(
  lit(1) >> 100,      // Returns int
  lit(2) >> "error",  // Returns const char*
  _ >> 0
);
```

> What the compiler says:
> "static_assert failed: all handlers must return the same type" or "no matching member function for call to 'apply'".

**Fix:** Ensure all branches return compatible types. Use explicit casts or `std::string` constructors if the types must differ.

```cpp
auto result = match(x) | on(
  lit(1) >> std::string("100"),
  lit(2) >> std::string("error"),
  _ >> std::string("0")
);
```
