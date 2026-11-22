---
hide:
  - navigation
---

# Patternia API Reference

## Overview

Patternia exposes one public matching shape:

```cpp
match(subject) | on(
  case_1,
  case_2,
  _ >> fallback
)
```

The pipeline is immediate.
There is no deferred builder stage.

---

## Match Expression {#match-expression}

### `match(subject)` {#matchsubject}

`match(subject)` creates the evaluation context for one subject value.

```cpp
int x = 7;
auto r = match(x) | on(
  lit(7) >> 1,
  _ >> 0
);
```

Rules:

- Lvalue subjects are borrowed without copying; built-in bindings
  remain const references.
- Temporary subjects and `std::move(subject)` are owned by the context.
  A non-const owned subject can be consumed by the selected handler.
- Cases are evaluated in source order.
- Matching uses first-match-wins semantics.

### Owning a subject {#owning-a-subject}

```cpp
auto task = std::make_unique<Task>();
match(std::move(task)) | on(
  $[([](const auto& p) { return p && p->ready(); })]
    >> [](std::unique_ptr<Task> p) { run(std::move(p)); },
  $ >> [](std::unique_ptr<Task> p) { defer(std::move(p)); }
);
```

Construction of the context moves `task` immediately. Patterns and
guards then observe the context's value through const references.
A rejected case leaves that value available to later cases. Only the
selected handler receives consuming bindings. The same rules apply
to structural members and variant alternatives, including nested
guards and `any`, `all`, and `neg` combinations.

The context owns its subject even if evaluation is delayed:

```cpp
auto pending = match(std::make_unique<Task>());
// The temporary is now stored in pending.
std::move(pending) | on(
  $ >> [](std::unique_ptr<Task> p) { run(std::move(p)); }
);
```

Constness is preserved. A const rvalue normally needs a copy to become
an owned subject, and its bindings remain const. Move-only mutable
callables are supported; for an owned match, a temporary or explicitly
moved `on(...)` pack also permits an `operator() &&` handler or a
move-only value on the right of `>>`. A stored lvalue case pack keeps
its handlers as lvalues for reuse.

Custom patterns retain their `bind(const Subject&)` contract. Values
materialized by a binding stay alive through the guard and handler,
and can move into the selected handler. References returned by custom
bindings retain their declared qualifiers; Patternia does not cast
away constness to consume them.

Compatibility note: this forwarding also applies to materialized
custom bindings in borrowed matches. If `bind()` returns `tuple<T>`
by value, a handler now receives `T&&`, rather than the previous
`T&`. Such handlers should accept `T`, `T&&`, or `const T&`.
Built-in borrowed bindings remain const references.

Owning match results are materialized by value before the context is
destroyed. A handler that deliberately returns a pointer, view, or
reference wrapper must still ensure that the referenced storage
outlives the result.

Owned subjects are dispatched like borrowed ones: dense literal lists
through one `switch`, `lit(...)` lists through the same compare-then-
switch selection, and variants with five or more alternatives through
a `switch` on `index()`. Other case lists, and variants with up to
four alternatives, are evaluated in source order. See
[Performance Tuning](guide/performance-tuning.md).

### `on(case1, case2, ...)` {#oncases}

`on(...)` groups the case list consumed by `match(subject)`.

```cpp
match(x) | on(
  lit(1) >> "one",
  lit(2) >> "two",
  _ >> "other"
);
```

Rules:

- The last case must be `_`, `$` or `$()` without a guard.
- Each entry must be a `pattern >> handler` case expression.
- Unreachable wildcard/alt ordering errors are diagnosed at compile time.

With two integer operands, `0 >> 1` is C++'s built-in bit shift,
not a case expression. `on(...)` rejects its result and suggests
`lit(0) >> 1`. Use `lit(0) >> func` for a function-pointer handler
as well: C++ rejects `0 >> func` before Patternia can validate it.

### `on<R>(case1, case2, ...)` {#on-result}

`on<R>(...)` fixes the result type of the match to `R` instead of
deducing the common type of the handlers' results. Each handler's
result must convert implicitly to `R`; the results do not need a
common type of their own.

```cpp
enum class Action { Start, Stop };
using Transition = std::variant<Started, Stopped>;

Transition next = match(action) | on<Transition>(
  val<Action::Start> >> Started{},
  _                  >> Stopped{}
);
```

Rules:

- Every handler is checked, including handlers of cases that the
  subject can never select. A handler whose result does not convert,
  a `void` handler with a non-`void` `R`, or a conversion that needs
  an `explicit` constructor is a compile-time error.
- `on<void>(...)` discards every handler's result.
- A reference `R` must bind directly to a reference the handler
  returns (`Derived &` to `const Base &` is fine). A handler that
  returns a value, or a reference that would first be converted to a
  temporary, is rejected because the result would dangle.
- `R` is not an array, function or abstract class type.
- `on<R>` changes only the result type. It does not supply a fallback
  or make a case list exhaustive, and it does not change how cases are
  dispatched. All rules of `on(...)` still apply.

`on<R>` works with borrowed and owned subjects, with `static_on`, and
with `const` or `constexpr` case packs. `PTN_ON(...)` always deduces
the result type; write `static_on([] { return on<R>(...); })` to cache
a pack with an explicit result type.

### `pattern >> handler` {#pattern-handler}

This is the core case-expression form.

```cpp
lit(1) >> 42
$(is<int>) >> [](int v) { return v * 2; }
$(has<&Point::x, &Point::y>) >> [](int x, int y) { return x + y; }
```

Handler forms:

- `pattern >> value`
- `pattern >> callable`
- `pattern >> expression`

Bindings are determined entirely by the pattern. A callable handler
that cannot take them but can be called with no arguments ignores
them: `$ >> [] { return 1; }` is valid.

### Expression handlers

An expression on the right of `>>` is evaluated only when its case
matches and its guard passes. It uses the same bound values as a
lambda handler. Arithmetic results keep their value type; comparisons
and logical expressions return `bool`. Expression handlers return by
value, so a bare placeholder does not leak a reference to a temporary
binding.

```cpp
match(n) | on(
  $[_ > 0] >> (_ * 2 + 1),
  $        >> -_
);

constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;
match(p) | on(
  $(pt)[x > 0] >> (x - y),
  $(pt)        >> (x * x + y * y),
  _            >> 0
);
```

- `_` refers to the first bound value. Use `$ >> _` to return the
  subject, or `$(is<int>) >> _ * 2` to compute from an alternative.
  An expression needs bindings: `_ >> _ + 1` is invalid because the
  wildcard on the left binds nothing.
- Member names resolve against the handler's pattern, including when
  it has guards. Reordering the listed members does not change what a
  name refers to. A member absent from the pattern is a compile error.
- Supported operators are `+`, `-`, `*`, `/`, `%`, unary `+` and `-`,
  comparisons, and the guard `&&` / `||` forms, with short-circuiting.
  As in guards, logical `!` is not provided.
- Parenthesize comparisons and logical expressions after `>>`, for
  example `$ >> (_ > 0)` or `$ >> (_ >= 0 && _ <= 10)`, because C++
  gives `>>` higher precedence than those operators. Binary `+` and
  `-` bind tighter than `>>`, so `$ >> _ + 1` is correct as written,
  but GCC's `-Wall` (`-Wparentheses`) suggests parentheses there;
  writing `$ >> (_ + 1)` keeps the build warning-free.
- Ordinary operands are stored by value when the case is built.
  Use a lambda for function calls or values that should be read from
  external state each time the handler runs, or pass the result on
  with [`| f`](#case-pipe).

### `to<T>` {#to}

`to<T>` is a callable that constructs a `T` from its arguments. As a
handler it receives the case's bindings, in order, and runs only when
the case is selected:

```cpp
struct Point { int x; int y; };
constexpr auto pt = has<&Point::x, &Point::y>;

match(p) | on(
  $(pt) >> to<Point>,      // Point{x, y}
  _     >> to<Point>       // Point{}
);

match(n) | on($ >> to<Celsius>);   // Celsius{n}
```

- An aggregate is initialized with braces, `T{args...}`; any other
  type with parentheses, `T(args...)`. `to<std::vector<int>>` called
  with `3` gives three elements, not one.
- `T` is a non-array, non-abstract object type without `const` or
  `volatile`.
- `to<T>` only passes the bindings on unchanged. To transform them
  first, write a lambda, or use `| to<T>` after an expression.
- `to<T>` takes whatever bindings the case has, like the lambda
  `[](auto &&...args) { return T{args...}; }`. If `T` cannot be built
  from them, that is a compile error, even when `T` has a default
  constructor: `$ >> to<std::string>` on an `int` does not give an
  empty string. Use `_ >> to<T>` for `T{}`.

### `case | f` {#case-pipe}

`p >> h | f` passes the result of handler `h` to the callable `f`,
once the case is selected. It is shorthand for a lambda that does the
same: the pattern, its bindings and the case's place in the list do
not change.

```cpp
match(n) | on(
  $[_ < 0] >> -_ | to<Celsius>,
  $        >> _ | parse | validate
);

match(std::move(job)) | on(
  $[_ == nullptr] >> to<Rejected>,
  $               >> _ | schedule      // the owned unique_ptr moves in
);

match(code) | on(
  lit(0) >> "ok" | to<std::string>,
  _      >> "error" | to<std::string>
);
```

- C++ gives `>>` higher precedence than `|`, so `| f` applies to the
  whole case: `$ >> _ * 2 | f` is `($ >> (_ * 2)) | f`.
- Any handler can come before `|`: an expression, a value, a lambda,
  `to<T>`, or another `| f`. Any callable can come after it: a
  function, a lambda, a pointer to member (`| &Point::y`) or `to<T>`.
  A pattern or a non-callable value after `|` is a compile error.
- `f` receives one value, the handler's result, and is checked
  against it at compile time even when the case is never selected. A
  `void` handler has no result to pass on.
- `f`'s result is the case's result. An rvalue reference result is
  returned by value, since it would refer into a temporary.
- `case | f` keeps the dispatch plan and the generated code of the
  equivalent lambda, and a pack whose handlers and callables are all
  stateless is still cached as `const` by `static_on`.

There is no form for several values (`f(x, y * 2)`): write a lambda.

---

## Wildcard {#wildcard}

### `_` {#_-wildcard}

`_` is the public wildcard and fallback pattern.

```cpp
match(x) | on(
  lit(1) >> "one",
  _ >> "other"
);
```

The wildcard does not bind values by itself.

---

## Literal Patterns {#literal-patterns}

### `lit(value)`

Runtime literal match.

```cpp
match(x) | on(
  lit(5) >> 42,
  _ >> -1
);
```

### `val<value>`

Compile-time literal match.
Useful when the literal is known at compile time and the lowering engine can
consider static dispatch.

```cpp
match(x) | on(
  val<1> >> 1,
  val<2> >> 2,
  _ >> 0
);
```

### `lit_ci(value)`

Runtime ASCII case-insensitive string match.

```cpp
match(s) | on(
  lit_ci("hello") >> 1,
  _ >> 0
);
```

---

## Binding Patterns {#binding-patterns}

### `$` and `$(...)`

Patternia keeps binding explicit.

```cpp
$               // bind the whole subject
$(subpattern)   // bind under a subpattern
```

Examples:

```cpp
match(x) | on(
  lit(0) >> -1,
  $ >> [](int v) { return v; }
);
```

An unguarded `$` (or its equivalent `$()`) can end the case list:
it matches every value and binds the whole subject. Borrowed subjects
bind by const reference; owned subjects follow the
[ownership rules](#owning-a-subject).
An existing trailing `_` remains valid. Guarded bindings such as
`$[_ > 0]` and bindings with subpatterns such as `$(is<int>)` still
need a fallback.

```cpp
match(v) | on(
  $(is<std::string>) >> [](const std::string &s) {
    return s.size();
  },
  _ >> 0
);
```

Binding rules:

- No pattern binds implicitly.
- Handler parameters follow binding order.
- Patterns that do not bind produce zero-argument handlers.

---

## Guard Attachment {#guard-attachment}

### `pattern[guard]`

Attach a guard to a binding pattern with `pattern[guard]`.

```cpp
match(x) | on(
  $[_ > 0 && _ < 10] >> "small",
  _ >> "other"
);
```

Guard evaluation order:

1. Match the pattern.
2. Bind values.
3. Evaluate the guard.
4. Invoke the handler if the guard passes.

A guard failure only rejects the current case.

---

## Guard Helpers {#guard-helpers}

### `_`

Placeholder alias for a single bound value.

```cpp
$[_ > 5]
```

Use `_` when one binding is enough and the predicate reads clearly without an
explicit name.

### Structured-binding member names

A `has<...>` pattern is tuple-like. Prefer a C++17 structured binding to name
its member placeholders without a macro:

```cpp
constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;

$(pt)[x * x + y * y == 25]
```

There is one name per non-ignored member, in the order listed in `has<...>`;
`_ign` slots do not get names. The names are placeholders, not values extracted
from a subject. `$(pt)` still controls which values reach the handler.

Inside a guard, names resolve to the position of their member in the `has<>`
member list at compile time, so they follow members, not positions — the order
of member pointers in a subsequent pattern does not matter:

```cpp
$(has<&Point::y, &Point::x>)[x == 3 && y == 4]  // x is still .x
```

A name whose member is not listed in `has<>` fails a static_assert. Member
names are only valid in guards attached to `has<...>`; using them on a
non-structural pattern is a compile-time error.

#### Scope and cached matchers

Function-local `auto [x, y] = pt;` works with `on(...)`, but not with
`PTN_ON(...)`: the macro uses a captureless lambda, which cannot access these
automatic bindings. For cached matchers, choose one of these options:

- Declare the pattern and its structured-binding names at namespace scope.
- In C++20 or later, give both the pattern and the bindings static storage
    inside the function:

    ```cpp
    static constexpr auto pt = has<&Point::x, &Point::y>;
    static auto [x, y] = pt;
    ```

- In C++17, keep `PTN_BIND` for function-local names used inside `PTN_ON`, or
  use `on(...)` instead. Any local pattern passed to `PTN_ON` must also be
  static; alternatively, write `has<...>` directly inside the macro.

### `PTN_BIND(Type, names...)` (deprecated)

`PTN_BIND` remains supported for compatibility, but prefer structured-binding
member names where the scope rules above permit them. It declares
`static constexpr member_t<&Type::name>` placeholders for one to ten
non-static data members:

```cpp
PTN_BIND(Point, x, y);

$(has<&Point::x, &Point::y>)[x * x + y * y == 25]
```

Declarations are valid at namespace or block scope. Their static storage
duration allows block-scope names inside `PTN_ON` even in C++17. A misspelled
member name fails at the `PTN_BIND` line.

### `rng(lo, hi, mode)`

Range helper for single-bound-value guards.

```cpp
$[rng(0, 10)]
$[rng(0, 10, pat::mod::open)]
```

Use callables for domain logic that does not read naturally as `_` or a short
named-placeholder expression.

### Migration from positional guard APIs

The guard surface now uses `_` for one bound value and structured-binding
member names for structural guards. The former `__`, `_0`, `arg<N>`,
`PTN_LET`, and `PTN_WHERE` spellings have been removed rather than retained as aliases.

```cpp
// Single value
$[_ > 0]

// Multiple structural members
constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;
$(pt)[x < y]
```

---

## Structural Matching {#structural-matching}

### `has<&T::member...>`

`has<>` describes structure.
Wrap it with `$(...)` to extract values.

```cpp
struct Point {
  int x;
  int y;
};

match(p) | on(
  $(has<&Point::x, &Point::y>) >> [](int x, int y) {
    return x + y;
  },
  _ >> 0
);
```

Properties:

- Member order is explicit and stable.
- Unlisted members are ignored.
- Validation happens at compile time.

---

## Variant Matching {#variant-matching}

### `is<T>`

Type-based `std::variant` match.

```cpp
match(v) | on(
  is<int> >> "int",
  $(is<std::string>) >> [](const std::string &s) {
    return "str:" + s;
  },
  _ >> [] { return std::string("other"); }
);
```

### `alt<I>`

Index-based `std::variant` match.

```cpp
match(v) | on(
  alt<0> >> "first",
  $(alt<1>) >> [](const std::string &s) {
    return "second:" + s;
  },
  _ >> "other"
);
```

Rules:

- `is<T>` requires `T` to appear exactly once.
- `alt<I>` requires `I` to be in range.
- Use `$(...)` when you want the alternative value bound into the handler.
  `$(is<T>)` and `$(alt<I>)` both bind the alternative, not the variant.
- `is<T>(p)` and `alt<I>(p)` also match the alternative against `p`. A plain
  value is treated as `lit(value)`: `is<int>(0)`, `alt<1>("hi")`.

---

## Pattern Combinators {#pattern-combinators}

### `any(ps...)`

Matches when at least one sub-pattern matches (OR semantics). Short-circuits
on the first matching sub-pattern.

```cpp
match(x) | on(
  any(val<1>, val<2>, val<3>) >> "1, 2, or 3",
  _ >> "other"
);
```

Properties:

- Non-binding: handlers receive zero arguments.
- Sub-patterns are evaluated left-to-right; evaluation stops at the first
  match.
- Requires at least one sub-pattern. Each argument is a pattern or a plain
  value (arithmetic, enumeration or string), which is treated as
  `lit(value)`: `any('a', 'e', 'i')`.
- Operator sugar: `(a || b)` is equivalent to `any(a, b)`, and `b` may be a
  plain value: `(lit(' ') || '\n')`. Note that `>>` binds tighter than
  `||`, so parenthesize: `(lit(1) || 2) >> handler`.

### `all(ps...)`

Matches only when every sub-pattern matches (AND semantics). Short-circuits
on the first mismatch.

```cpp
match(x) | on(
  all(any(val<1>, val<2>), val<2>) >> "2",
  _ >> "other"
);
```

Properties:

- Non-binding: handlers receive zero arguments.
- Sub-patterns are evaluated left-to-right; evaluation stops at the first
  mismatch.
- Requires at least one sub-pattern. Each argument is a pattern or a plain
  value, which is treated as `lit(value)`: `all(neg(0), neg(1))`.
- Operator sugar: `(a && b)` is equivalent to `all(a, b)`, and `b` may be a
  plain value. Note that `>>` binds tighter than `&&`, so parenthesize:
  `(p && q) >> handler`.

### `neg(p)`

Negates the match result of a sub-pattern. `neg(p)` matches when `p` does
not match, and vice versa.

```cpp
match(x) | on(
  neg(val<0>) >> "non-zero",
  _ >> "zero"
);
```

Properties:

- Non-binding: handlers receive zero arguments.
- Accepts exactly one sub-pattern (no zero- or multi-argument form). A plain
  value is treated as `lit(value)`: `neg(0)`.
- `neg(neg(p))` restores the original match behavior (double negation cancels).
- Operator sugar: `!p` is equivalent to `neg(p)` and needs no parentheses:
  `!val<200> >> "error"`.

The pattern-level operators need a pattern on the left, and a pattern or a
plain value on the right, so they never collide with the guard-level `&&` /
`||` (which keep their `pred_and` / `pred_or` meaning inside `[...]`
guards).

A `bool` on the right is a plain value like any other: it is compared with
the subject, not tested as a condition. `(neg(0) && flag)` is
`all(neg(0), lit(flag))`, so it matches a subject equal to `flag`, not every
non-zero subject while `flag` is true. Use a guard to test a condition.

---

## Cached Case Packs {#cached-case-packs}

### `static_on(...)`

Cache a stateless `on(...)` factory.

```cpp
match(x) | static_on([] {
  return on(
    val<1> >> 1,
    val<2> >> 2,
    _ >> 0
  );
});
```

The cases are built on the first call and cached in a static. When
no handler keeps state (a captureless lambda, a constant such as
`>> 1`, or an expression such as `>> _ + 1`), the cache is `const`,
so the compiler can treat the cases as constants. `static_on` then
returns a `const` reference. A handler with state, such as a function
object with a counter, keeps a mutable cache, and its state persists
between calls.

### `PTN_ON(...)`

Convenience macro over `static_on(...)`, with the same caching rules.

```cpp
match(x) | PTN_ON(
  val<1> >> 1,
  val<2> >> 2,
  _ >> 0
);
```

The factory must be stateless. When the cases can be evaluated at
compile time, the cache is initialized as a constant and needs no
initialization guard.

### Constant case packs

A case pack may also be `const` or `constexpr` and reused across
matches:

```cpp
constexpr auto classify = on(val<1> >> 1, val<2> >> 2, _ >> 0);

int a = match(x) | classify;
int b = match(compute()) | classify;
```

Handlers of a `const` case pack are called through a `const`
reference.

---

## Namespace Summary {#namespace-summary}

The public surface is available through `namespace ptn`, plus the macros listed
below:

- `match`
- `on`, `on<R>`
- `to<T>`, `case | f`
- `lit`, `val`, `lit_ci`
- `$`
- `_`, `rng`
- `has`
- `is`, `alt`
- `any`, `all`, `neg`
- `PTN_ON` (macro)
- `PTN_BIND` (deprecated compatibility macro)

---

## Minimal Example

```cpp
#include <ptn/patternia.hpp>

int main() {
  using namespace ptn;

  int x = 2;
  int r = match(x) | on(
    lit(1) >> 10,
    lit(2) >> 20,
    _ >> 0
  );

  return r == 20 ? 0 : 1;
}
```
