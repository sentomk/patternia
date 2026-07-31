# Reflection-Named Binding — Experimental Design

> Branch: `experimental/reflection-named-binding`
>
> Status: exploration; not production-ready or API-stable.

## API invariant

Reflection extends Patternia's existing structural API. It does not introduce a
parallel namespace or a second binding abstraction.

```cpp
struct Point {
  int x;
  int y;
};

// C++17
$(has<&Point::x, &Point::y>)
    >> [](int x, int y) { return x + y; }

// C++26 reflection
$(has<^^Point::x, ^^Point::y>)
    >> [](int x, int y) { return x + y; }
```

The only structural spelling change is `&T::member` to `^^T::member`.
`has<>` remains non-binding and `$()` remains the binding operator. There is no
`reflect::has`, `reflect::bind`, `has_refl`, or reflection-only factory.

Reflected members may be selected, omitted with `_ign`, or reordered exactly as
member pointers can be on the C++17 path:

```cpp
$(has<^^Point::y, ^^Point::x>)
    >> [](int y, int x) { return y - x; }
```

## Named guard target

The intended reflection experience removes the manual C++17 declaration:

```cpp
PTN_BIND(Point, x, y);
```

and ultimately permits member names in a guard without repeating them:

```cpp
$(has<^^Point::x, ^^Point::y>)[x*x + y*y == 25]
```

P2996 alone cannot implement this syntax. The guard expression is parsed before
`has<...>` is instantiated, so the unqualified identifiers `x` and `y` must
already exist in the caller's lexical scope. Reflection can recover their names
and splice member access, but an ordinary library template cannot inject local
declarations retroactively.

Consequently, the branch must not claim automatic caller-scope injection until
it has a token-level implementation. The intended implementation boundary is a
future procedural `PTN_ON!` macro that receives each complete case before C++
parsing, derives names from `has<^^...>`, and emits a private declaration scope
for that case. P3294 token sequences and scoped macros are the standards-track
mechanism for this design; the current Bloomberg P2996 Clang fork does not
implement them.

Until such a frontend or procedural-macro prototype exists, positional guards
remain executable:

```cpp
$(has<^^Point::x, ^^Point::y>)[_0*_0 + arg<1>*arg<1> == 25]
```

This is an implementation limitation, not an alternative public design.

## Compilation

The current reflection path requires the Bloomberg P2996 Clang fork:

```bash
clang++ -std=c++26 -freflection \
  -nostdinc++ -I<clang-p2996/libcxx/include> \
  --stdlib=libstdc++ -I<patternia/include> example.cpp
```

Including `ptn/patternia.hpp` is sufficient. No experimental reflection header
is required.

## Compatibility

The reflection implementation is conditionally compiled behind
`__has_feature(reflection)`. Existing C++17 through C++26 builds without the
reflection extension retain member-pointer behavior and the same public API.

## References

- [P2996R13](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2996r13.html)
- [P3294R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3294r2.html)
- [Bloomberg Clang P2996](https://github.com/bloomberg/clang-p2996)
