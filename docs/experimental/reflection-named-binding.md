# Reflection-Named Binding — Experimental Design

> Branch: `experimental/reflection-named-binding`
>
> Status: **Exploration** — not production-ready, not API-stable.

## Problem

Patternia's guard syntax requires positional placeholders (`_0`, `_1`) because
C++17 pattern matching produces unnamed tuple bindings. This is the root cause
of all guard ergonomics issues:

```cpp
// Current (C++17): positional, ugly, no semantic meaning
$(has<&Point::x, &Point::y>)[_0*_0 + _1*_1 < 25]
```

## Goal

Use C++26 reflection (P2996) to let `$` automatically discover struct members
and expose them as **named bindings** in guard expressions:

```cpp
// Target (C++26): named, readable, matches Rust/Scala ergonomics
$[x*x + y*y < 25]
```

## How It Works

### 1. `$` reflects the subject type

```cpp
// When $ matches a Point, it calls:
nonstatic_data_members_of(^^Point, access_context::current())
// → [^^Point::x, ^^Point::y]
```

### 2. Each member becomes a named placeholder

For each reflected member, generate a compile-time placeholder bound to that
member's reflection. The placeholder carries the member's name (via
`identifier_of`) and its index in the member list.

### 3. Guard expressions use member names directly

The named placeholders participate in the existing expression template system
(`operator>`, `operator+`, etc.) — they're `arg_t<N>` under the hood, but the
user writes `x` instead of `_0`.

### 4. Splice access at match time

When the guard is evaluated, each placeholder resolves to the actual member
value via splice: `subject.[:member_reflection:]`.

## API Surface

```cpp
#if __has_feature(reflection) || defined(PTN_EXPERIMENTAL_REFLECTION)

namespace ptn::pat::reflect {

  // Reflective bind: auto-discovers struct members
  template <typename T>
  struct decompose_pattern;

  // Factory: creates a reflective binding pattern
  template <typename T>
  constexpr auto decompose();

  // Enhanced $ that uses reflection when available
  // Falls back to existing behavior for non-struct types
  constexpr struct dollar_t {} $ {};

} // namespace ptn::pat::reflect

#endif
```

## Usage Examples

```cpp
struct Point { int x; int y; };
struct Packet { uint8_t type; uint16_t length; };

// Named guard — no _0, _1 needed
match(point) | on(
    $[x*x + y*y < 25] >> [](auto x, auto y) { return x + y; },
    $ >> [](auto p) { return 0; }
);

// Mixed: named guard with type check
match(pkt) | on(
    $(is<Packet>())[type == 0x01 && length > 0]
    >> [](auto type, auto length) { return decode(type, length); }
);
```

## Compilation Requirements

- Clang P2996 fork (`bloomberg/clang-p2996`, `p2996` branch)
- Flags: `-std=c++26 -freflection`
- Feature guard: `#if __has_feature(reflection)`

## Non-Goals

- Does not replace C++17/20/23 guard syntax. The `_0`/`_1` expression template
  system remains the fallback for compilers without reflection.
- Does not require reflection for basic pattern matching. Only the `$` named
  binding feature uses it.

## Open Questions

1. **Name injection mechanism**: How to make `x` and `y` visible in the guard
   expression scope? Options:
   - Macro-based injection (fragile, ugly)
   - ADL with named placeholder types (cleaner, but requires careful namespace design)
   - `consteval` block generating local variables (needs P3289 consteval blocks)

2. **Fallback behavior**: When `$` is used on a type without reflection (e.g.,
   `int`), should it fall back to whole-value binding (current `$` behavior)?

3. **Partial decomposition**: Should `$[x, y]` allow selecting specific members
   instead of all?

## References

- [P2996r13](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2996r13.html) — Reflection for C++26
- [bloomberg/clang-p2996](https://github.com/bloomberg/clang-p2996) — Reference implementation
- Patternia PRD §7 — Architecture (reflection-aware `$` evolution path)
