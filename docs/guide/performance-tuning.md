## Performance Tuning

### When to use `PTN_ON(...)` macro
The `PTN_ON` macro caches the case pack in a function-local static. This ensures zero construction overhead on hot paths, as the matcher object is only built once during the first execution.

Use this when the same case pack is called repeatedly in a loop or a frequently invoked function. It provides the best performance for local, repeated matching logic.

Example:
```cpp
int classify(int x) {
  return match(x) | PTN_ON(
    lit(0) >> 0, lit(1) >> 1, lit(2) >> 2,
    __ >> -1
  );
}
```

**Caveat:** Lambdas inside `PTN_ON` must be stateless (no captures), as they are stored in a static context.

### When to use `static_on(factory)`
The `static_on` function creates a persistent static match object from a factory lambda. It serves as a more explicit version of the caching mechanism used by `PTN_ON`.

Use this when you need to store the match object separately from a single call site, such as pre-computing a matcher object at initialization time and then calling it from multiple different locations.

Example:
```cpp
auto get_matcher() {
  return static_on([] {
    return on(
      lit("start") >> Action::Start,
      lit("stop") >> Action::Stop,
      __ >> Action::Unknown
    );
  });
}

void process(const std::string& cmd) {
  auto result = match(cmd) | get_matcher();
}
```

### When raw `match(x) | on(...)` is fine
The raw pipeline form constructs the case objects on every call. While this adds a small amount of overhead, Patternia's dispatch system is highly optimized and fast enough for most scenarios without explicit caching.

Use the raw form when:
- The call is not on a hot path.
- The overhead is negligible (small case packs or infrequent calls).
- You need to capture local variables in your handlers.

### Dispatch tiers and their effect
Patternia uses different dispatch strategies based on the patterns provided:

- **Literal Dense/Runtime Dense:** Used for contiguous literal values. These are lowered to efficient jump tables or direct indexing.
- **Variant Inline/Segmented/Compact:** Optimized strategies for `std::variant`. Inline dispatch is used for small variants, while segmented or compact forms handle larger or more complex type distributions.

According to `docs/assets/bench/latest.md`, Patternia is the fastest in 3 out of 4 tested scenarios. It is particularly efficient in `VariantMixed` tests (~0.94ns per call). The slowest scenario is `PacketMixed`, where Patternia is +2.68% vs a manual Switch statement, primarily due to structural binding overhead. Literal-only scenarios typically run at approximately 1.1ns per call.
