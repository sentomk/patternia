---
hide:
  - navigation
---

# Roadmap

This page lists the features planned for upcoming Patternia releases,
grouped by the version they are expected to ship in. It describes
direction, not a promise: scope, names and syntax can still change
before a release, and a feature can move to a later version.

Syntax shown for unreleased features is the current design. A section
marked implemented is available on the development branch; other
examples are proposals, not APIs available today.

Each version has one status:

| Status | Meaning |
|---|---|
| **Released** | Shipped. See the linked release note. |
| **In progress** | Being designed or implemented now. |
| **Planned** | Scheduled for this version, work not started. |
| **Exploring** | Not scheduled. Needs a design decision or user demand first. |

Feedback on any item is welcome in
[GitHub issues](https://github.com/sentomk/patternia/issues).

---

## Delivery sequence

| Version | Main additions |
|---|---|
| v0.9.6 | Shorter cases, move-aware matching, and binding/copy performance work |
| v0.9.7 | Explicit result types and lazy construction/function expressions |
| v0.10 | Exhaustiveness, checked intervals, and complete enum domains |
| v0.11 | Nested patterns, multiple subjects, and structural coverage |
| v0.12 | Broader performance work, reflection, and positional constructor patterns |

Move-aware binding comes before lazy expressions; finite-domain
coverage comes before the nested pattern-matrix analysis. These are
delivery dependencies, not claims that the planned APIs exist today.

---

## v0.9.6 — Shorter cases, moves, and performance {#v096-shorter-cases data-toc-label="v0.9.6" }

**Status:** In progress

The goal of this release is to remove boilerplate that most matches
carry today: an unreachable `_` after a case that already matches
everything, single-expression lambdas, and `lit(...)` inside other
patterns. It also adds ownership-aware matching and targets avoidable
copies and repeated binding. Existing borrowed, read-only matches
keep their meaning; consuming a subject is explicit.

### `$` as the fallback

Implemented; pending the v0.9.6 release.

`$` can end a case list on its own, instead of being followed by an
unreachable `_`:

```cpp
match(roll) | on(
  lit(3) >> add_fancy_hat,
  lit(7) >> remove_fancy_hat,
  $      >> [](int r) { move_player(r); }   // already the fallback
);
```

The equivalent `$()` form also works. A guard or subpattern still
requires a fallback, and existing matches with a trailing `_` remain
valid.

v0.10 extends this to every pattern that matches every value, as part
of exhaustiveness checking.

### Expression handlers

Implemented; pending the v0.9.6 release.

The right-hand side of `>>` accepts the same expressions as guards, so a
handler that only computes a value no longer needs a lambda:

```cpp
constexpr auto pt = has<&Point::x, &Point::y>;
auto [x, y] = pt;

match(p) | on(
  $(pt)[x > 0] >> x - y,
  $(pt)        >> x * x + y * y,
  _            >> 0
);

match(n) | on(
  $(is<int>) >> _ * 2 + 1,
  _          >> 0
);
```

The first version covers arithmetic, comparison and logical guard
expressions, plus unary `+` and `-` in both handlers and guards.
Comparisons and logical expressions after `>>` need parentheses,
such as `$ >> (_ > 0)`. Calling functions inside an expression handler
is out of scope; use a lambda for that.

### Plain values inside patterns

Implemented; pending the v0.9.6 release.

Wherever a pattern is expected as an argument, a plain value is treated
as `lit(value)`. This extends the existing `0 >> handler` shorthand to
nested positions:

```cpp
// today
any(lit('a'), lit('e'), lit('i')) >> "vowel",
is<int>(lit(0))                   >> "int zero",

// v0.9.6
any('a', 'e', 'i') >> "vowel",
is<int>(0)         >> "int zero",
(lit(' ') || '\n') >> "whitespace",
```

It applies to the arguments of `any`, `all` and `neg`, to the
subpatterns of `is<T>(...)` and `alt<I>(...)`, and to the right
operand of `||` and `&&`. A plain value is a runtime literal; use
`val<V>` where static dispatch matters.

### Move-aware matching

Implemented; pending the v0.9.6 release.

Accept temporary and explicitly moved subjects, including move-only
values. The match context owns an rvalue subject for its lifetime;
matching an lvalue continues to borrow it without copying.

```cpp
auto task = std::make_unique<Task>();

match(std::move(task)) | on(
  $ >> [](std::unique_ptr<Task> owned) { enqueue(std::move(owned)); }
);
```

The ownership contract is:

- Borrowed lvalues keep today's const-reference bindings. This release
  does not silently make existing handlers mutate their subjects.
- A non-const rvalue is owned by the context and can be forwarded to
  the selected handler. Moving into the context may consume the source
  immediately; rejected cases must not consume the context's value.
- Pattern tests and guards only observe bindings. A failed guard or
  failed arm of a combinator leaves the owned value available to later
  cases. Forwarding for consumption happens only after a case wins.
- Constness is preserved. Matching a const rvalue does not grant
  permission to move from a const object as if it were mutable.
- Structural and variant bindings preserve the ownership policy of
  the subject. A custom binding that materializes a value must keep it
  alive through the guard and handler. Results must not accidentally
  reference storage destroyed with the match context.

Acceptance tests cover move-only subjects and handlers, rejected
guards followed by a consuming fallback, combinator short-circuiting,
const subjects, temporary lifetimes, and copy/move counts. User
handlers remain responsible for references they deliberately return.

Owned subjects currently use source-order evaluation. Move-only
callables and materialized bindings are supported; measuring and
optimizing the owning path remains part of the performance work below.

### Performance work

Planned alongside move semantics, rather than deferred to v0.12:

- Remove avoidable subject, case-pack, and handler copies, including
  support for move-only callables in one-shot matches.
- Avoid repeated extraction/binding on the selected guard/handler
  path where the binding contract permits it; do not change predicate
  order, invocation counts, or first-match-wins behavior to do so.
- Measure owning and borrowed matches, rejected guards, structural
  bindings, and generic versus optimized dispatch. Keep semantic
  tests separate from timing benchmarks.
- Measure continuation stack depth and temporary binding lifetimes
  across rejected owned guards before flattening their evaluation.
- Record compile time and binary size as well as run time. Publish
  reproducible before/after measurements; do not promise a speedup
  before measuring it.

Broader dispatch optimization and benchmarks for nested matching stay
in v0.12. Optimizations must preserve the new ownership contract.

### Fixes and diagnostics

- Implemented: `$(pred(...))` binds the matched value, like
  `$(is<T>)` does.
- Implemented: `on(...)` rejects non-case arguments before handler
  deduction and dispatch planning. For `0 >> 1`, the diagnostic
  explains C++'s built-in bit shift and suggests `lit(0) >> 1`.
  C++ rejects `0 >> func` itself when `func` is a function pointer;
  use `lit(0) >> func` to select Patternia's operator.
- Bugs reported in
  [GitHub issues](https://github.com/sentomk/patternia/issues) with the
  `bug` label before the release.

---

## v0.9.7 — Result types and lazy expressions { data-toc-label="v0.9.7" }

**Status:** Planned

Reduce repeated result wrappers and single-expression lambdas without
hiding complex business operations inside the DSL.

### Explicit result type

`on<R>(...)` fixes the match result type once. Every selected handler
result must be convertible to `R`; deduced `on(...)` remains available.

```cpp
enum class Action { Start, Stop };
using Result = std::variant<Started, Stopped>;

match(action) | on<Result>(
  val<Action::Start> >> Started{},
  _                 >> Stopped{}
);
```

Result typing is independent of coverage: `on<R>` neither supplies a
fallback nor makes a partial match exhaustive. Invalid branch results
must be diagnosed even if that branch is not selected at run time.

### Lazy construction and calls

`make<T>(expressions...)` constructs `T{...}` after its case wins.
`call(f, expressions...)` invokes a callable at that same point. Both
compose with existing arithmetic and named binding expressions.

```cpp
struct Point { int x; int y; };

match(n) | on($ >> make<Point>(_, _ * 2));
match(text) | on($ >> call(parse, _));
```

An ordinary value handler is still eager: `p >> build()` evaluates
`build()` while constructing the case pack; `p >> call(build)` waits
until that case is selected. Unselected expression handlers perform
no calls or result construction.

These expressions use v0.9.6's ownership rules. Reads may reuse a
binding, but expression forwarding must not silently consume the same
bound value twice. Define argument evaluation order and diagnose
unsupported repeated consumption before freezing the API. Complex
control flow and ambiguous ownership remain ordinary lambda code.

---

## v0.10 — Exhaustiveness checking { data-toc-label="v0.10" }

**Status:** Planned

The goal of this release is to let the compiler check that a match
covers every case, and to reject cases that can never be selected.
Two distinct pipeline stages provide domains that cannot be inferred
from the subject type: `within<L, H>` for an integer interval and
`domain<E>` for a complete enum value set.

A match no longer needs a trailing `_` when its cases cover every
possible value of the subject:

```cpp
using Shape = std::variant<Circle, Square, Triangle>;

double area(const Shape &s) {
  return match(s) | on(
    $(is<Circle>)   >> [](const Circle &c) { return c.area(); },
    $(is<Square>)   >> [](const Square &q) { return q.area(); },
    $(is<Triangle>) >> [](const Triangle &t) { return t.area(); }
  );   // no `_` needed
}
```

If a case is missing, for example the `Triangle` one, the match no
longer compiles, and the error names what is missing:

```text
[Patternia.on]: non-exhaustive match on std::variant<Circle, Square, Triangle>.
Missing alternatives: Triangle.
Tip: add the missing cases, or end the list with '_ >> handler'.
```

A trailing `_` or `$` after a complete list stays allowed, so existing
code keeps compiling.

### Subjects that can be checked

| Subject | Covered when the cases include |
|---|---|
| `std::variant<Ts...>` | every alternative, by type or by index |
| `bool` | `val<true>` and `val<false>` |
| Enumeration with complete compile-time metadata | `val<E::x>` for every distinct enumerator value |
| Integer constrained by `within<L, H>` | static literals/ranges covering every value in `[L, H)` |
| Any subject | a case that matches every value, such as `_` or `$` |

An enumeration can be checked when it has a fixed underlying type of
one byte, such as `std::uint8_t`. Nothing else has to be written:

```cpp
enum class Coin : std::uint8_t { Penny, Nickel, Dime, Quarter };

int value_in_cents(Coin c) {
  return match(c) | on(
    val<Coin::Penny>   >> 1,
    val<Coin::Nickel>  >> 5,
    val<Coin::Dime>    >> 10,
    val<Coin::Quarter> >> 25
  );   // complete, no `_` needed
}
```

C++17 cannot list the enumerators of an enumeration, so Patternia finds
them at compile time: it tries every value of the underlying type (256
for an 8-bit type, two for `bool`) and keeps the ones the compiler
names as enumerators. Every value is tried, so no enumerator can be
missed, and the list follows the enumeration as it changes. Adding
`HalfDollar` to `Coin` makes every match on `Coin` without `_` fail to
compile until it handles the new value.

Detection relies on how the compiler prints enumeration values. If
Patternia does not recognize a compiler's output, detection is turned
off and these enumerations need `_`, as they do today. Detection never
reports a match as complete when it is not.

The cost is portability: a match on `Coin` without `_` that compiles
with one compiler fails to compile with a compiler whose output is not
recognized. Support is planned for the compilers in the CI matrix (GCC,
Clang, MSVC and clang-cl), and the release notes will list the ones
verified. Code that must build everywhere should keep `_` until all of
its compilers are listed.

One-byte detection is an optional metadata source, not an API
requirement. Ordinary `enum class E` with the default `int` underlying
type is supported through a complete compile-time metadata provider:
an adapter to existing enum metadata or an explicit enumerator list.
The customization spelling is to be finalized during implementation.
Do not require applications to change an enum's representation to use
Patternia.

An explicit list is a completeness contract maintained by its author;
C++17 cannot detect a newly declared enumerator omitted from that
list. Automatic detection and reflection avoid that maintenance gap.
With C++26 reflection, complete metadata no longer needs registration;
see [v0.12](#v012-performance-and-c26-reflection).

Without reliable metadata, an ordinary enum match still needs a
fallback, and requesting `domain<E>` is a compile-time error naming
the missing metadata. Scanning part of a wide underlying type never
counts as complete discovery.

Unconstrained integers, floating-point values and strings still need
a total fallback such as `_` or `$` in this release.

### Checked integer intervals: `within<L, H>`

This stage restricts an integral subject to the half-open interval
`[L, H)` and preserves its value type. Bounds are compile-time values;
empty, reversed, or incompatible bounds are rejected.

```cpp
match(n % 3u) | within<0u, 3u> | on(
  val<0u> >> "zero",
  val<1u> >> "one",
  val<2u> >> "two"
);
```

The companion static pattern `range<L, H>` matches an integer
interval, so a large domain does not need one case per value:

```cpp
match(score) | within<0, 101> | on(
  range<0, 60>   >> "retry",
  range<60, 101> >> "pass"
);
```

`within` constrains the input; `range` is a case pattern. Both use
half-open bounds. Existing `rng(lo, hi)` remains a runtime predicate
with its existing range modes and does not prove static coverage.
Coverage uses interval operations, not one template instantiation
per integer. Signedness and boundary checks must not narrow or wrap
values before deciding whether they are in range.

### Complete enum domains: `domain<E>`

This stage validates membership in all declared values of `E`, then
supplies an `E` subject to the case pack. It accepts enum or integral
input and requires complete metadata for the target enum.

```cpp
enum class Status { Ok = 0, Retry = 10, Failed = 100 };
// Supply complete enum metadata on C++17, or use C++26 reflection.

match(code) | domain<Status> | on(
  val<Status::Ok>     >> handle_ok,
  val<Status::Retry>  >> retry,
  val<Status::Failed> >> handle_failure
);
```

The domain above is `{0, 10, 100}`, not the interval `[0, 101)`.
Aliases with equal values count once. Membership is checked before
conversion to the enum's underlying type: a large integer must not
wrap into a valid enumerator. No particular underlying type, including
`std::uint8_t`, is required by `domain<E>`.

For a subject already of type `E`, `match(e) | on(...)` can infer the
domain from the same metadata without an explicit stage. An explicit
`domain<E>` additionally enforces membership before case evaluation.

### Domain contracts and pipeline boundaries

Both stages validate input before any case or handler runs. A value
outside the declared interval/set is a runtime error, even if the
case pack has `_`; the fallback covers the declared domain, not a
broken domain contract. Error handling must also be specified for
builds with exceptions disabled before the release.

The proof's domain comes from the type or an independent declaration,
never from the case list itself: inferring `{0, 1}` from cases for 0
and 1 would hide a missing case for 2. A guard cannot serve as an
unchecked assertion about the domain.

`on(...)` remains the terminal stage that sees the subject, its domain,
and the complete case list. No extra `end` stage is needed. Domain
stages belong between `match(...)` and `on(...)`.

Acceptance tests cover missing values, overlapping static intervals,
enum aliases, absent metadata, negative/unsigned boundaries, and
integers that would become valid only after truncation. They also
verify that a domain violation runs no selected handler and cannot be
hidden by `_`, and that guards do not create false coverage proofs.

### Patterns that count as coverage

- `is<T>` and `alt<I>`, with or without `$(...)`.
- `is<T>(p)` and `alt<I>(p)` when `p` matches every value of that
  alternative, such as `_`, `$`, or `has<...>` with no member
  subpatterns or with member subpatterns that all match every value.
- `val<V>` for `bool` and for enumerations that can be checked.
- `val<V>` and `range<L, H>` cover integer values inside a domain
  established by `within`. Interval unions can prove full coverage.
- `any(...)` and `||` cover the union of what their operands cover, so
  `(is<Square> || is<Triangle>)` covers both alternatives.

Cases with a guard, and patterns whose result is only known at run
time, such as `lit(...)`, `pred(...)` and `rng(...)`, never count as
coverage. A plain value on the left of `>>`, as in `Coin::Penny >> 1`,
is a runtime literal, so it does not count either; write
`val<Coin::Penny>` instead.

### Redundant cases

A case that can never be selected because earlier cases already cover
everything it matches becomes a compile-time error. This extends the
current check for a repeated `alt<I>` to `is<T>`, combinators, and any
case written after a complete list. The one exception is a trailing
`_` or `$`, which stays allowed.

This is a breaking change: code that compiles today with an
unreachable case stops compiling. The release notes will list it, and
the error names the case to delete.

Enumerators that share a value are one case. If `E::Alias` has the
same value as `E::A`, then `val<E::A>` and `val<E::Alias>` match the
same value: either one covers it, and writing both is a redundant case.

### When no case matches at run time

Two kinds of values can reach a complete match without matching any of
its cases:

- A `std::variant` that is valueless after an exception during
  assignment. In a match without `_`, it throws
  `std::bad_variant_access`, the same as `std::visit`, or calls
  `std::abort` when exceptions are disabled.
- An enumeration value that is not an enumerator, such as
  `static_cast<Coin>(200)`. In a match without `_`, it is a run-time
  error and never falls through silently. Whether it throws or aborts
  will be decided before the release.

Without an explicit domain stage, a match with `_` sends both to `_`,
as it does today. An explicit `domain<E>` instead rejects an invalid
enum value before dispatch, even when `_` is present.

Flag-style enumerations routinely hold values that are not
enumerators. With
`enum class Perm : std::uint8_t { Read = 1, Write = 2, Exec = 4 }`, a
combined value such as `Read | Write` is one of them, so match
flag-style enumerations with `_`.

---

## v0.11 — Nested and multi-subject matching { data-toc-label="v0.11" }

**Status:** Planned

The goal of this release is to match on the shape of data: members
with their own patterns, tuple elements, optional values, and several
subjects at once. The new patterns ship as an experimental API for one
minor version before their syntax is frozen.

### Patterns on members and elements

`has<...>` takes one subpattern per listed member, and `at<...>`
projects a single member or element before matching it:

```cpp
match(p) | on(
  has<&Point::x, &Point::y>(0, 0) >> "origin",
  has<&Point::x, &Point::y>(_, 0) >> "on x axis",
  at<&Point::y>(0)                >> "on x axis",   // same as above
  _                               >> "elsewhere"
);
```

`at<I>(p)` does the same for the `I`-th element of a tuple, pair or
array. Patterns nest to any depth, and the bindings of the
subpatterns are concatenated in the order they are written.

### Tuple patterns and multiple subjects

`tup(...)` matches a tuple-like subject element by element, and
`match(a, b, ...)` matches several subjects together:

```cpp
for (int i = 1; i <= 100; ++i) {
  match(i % 3, i % 5) | on(
    tup(0, 0) >> [] { std::puts("fizzbuzz"); },
    tup(0, _) >> [] { std::puts("fizz"); },
    tup(_, 0) >> [] { std::puts("buzz"); },
    _         >> [i] { std::printf("%d\n", i); }
  );
}
```

Multiple subjects extend the ownership rules introduced in v0.9.6:
each lvalue is borrowed and each rvalue is owned for the duration of
the match. Nested bindings preserve that policy. Failed subpatterns
and guards must leave owned values available to later cases.

### `some` / `none`

Patterns for the two states of `std::optional`, raw pointers and smart
pointers:

```cpp
std::optional<int> v = find(id);

match(v) | on(
  some(0)        >> 0,
  $(some)[_ < 0] >> [](int x) { return -x; },
  $(some)        >> [](int x) { return x; },
  none           >> -1
);
```

### Names for nested positions

Structured-binding names extend to nested patterns. A name is anchored
to its full path, so the same member appearing twice gets two
different names:

```cpp
constexpr auto start = at<&Line::a>($(has<&Point::x, &Point::y>));
auto [x, y] = start;

match(line) | on(
  start[x * x + y * y == 25] >> "starts on the circle",
  _                          >> "elsewhere"
);
```

### Exhaustiveness for nested and multi-subject matches

Exhaustiveness checking extends to `tup(...)`, `some` / `none` and
nested variants, using a pattern-matrix algorithm. A missing
combination is reported at compile time. For very large matches the
check stops at a fixed limit and asks for `_` instead.

---

## v0.12 — Performance and C++26 reflection { data-toc-label="v0.12" }

**Status:** Planned

### Broader performance work

This extends the ownership and binding work scheduled for v0.9.6.

- Benchmarks for the default and call spellings, variants with 9 to 15
  alternatives, binding patterns, and nested and multi-subject matches.
- Investigate the scenarios that are currently slower than
  `std::visit` or a hand-written `switch`, and optimize the common
  shapes of the new nested patterns.
- Publish the benchmark scripts and method so that "not slower than
  `std::visit`" is reproducible.

### C++26 reflection mode (experimental)

When the compiler supports C++26 static reflection, the same structural
API accepts reflections in place of pointers to members:

```cpp
constexpr auto pt = has<^^Point::x, ^^Point::y>;
auto [x, y] = pt;
```

Reflection mode also enables:

- complete enum metadata for any underlying type without registration,
  shared by ordinary matches and `domain<E>`,
- positional destructuring of aggregates without listing members.

Reflection mode is off by default and does not change behavior in
C++17 or C++20.

### Constructor-style patterns (experimental)

`of<T>(patterns...)` is the candidate spelling for matching a type and
its positional fields together. For a variant it first selects `T`;
for a direct `T` subject it destructures that object. It lowers to the
same structural patterns, bindings, and coverage analysis as `has`.

```cpp
match(event) | on(
  (is<Quit> || of<Key>('q', true)) >> quit,
  of<Key>($(any('h', 'j', 'k', 'l')), false) >> navigate,
  (is<Resize> || is<Expose>) >> redraw,
  _ >> ignore
);
```

Here `Key` has fields `code` and `ctrl` in that order; `navigate`
receives the bound character. Field order is part of the pattern's
contract. Prefer named member patterns for larger or evolving types.

C++26 reflection supplies aggregate decomposition. A C++17 path may
use an explicit decomposition adapter; its protocol must be settled
before shipping it. Unregistered, unrestricted aggregate reflection
is not promised for C++17. Keep this syntax separate from `is<T>(p)`,
whose single subpattern continues to match the whole alternative.

---

## v1.0 — Stable API { data-toc-label="v1.0" }

**Status:** Planned

- API freeze, with a migration guide from 0.x.
- Removal of the deprecated `PTN_BIND`, or a final decision to keep
  it.
- Full compiler matrix in CI: GCC, Clang and MSVC, including an MSVC
  C++17 configuration and macOS with Apple Clang.
- No benchmarked `std::variant` scenario slower than `std::visit`.

---

## Exploring

Not scheduled for any version. Each needs a design decision or a
concrete use case first.

- **Single-pattern test.** `matches(x, p)` returns `bool` without a
  case list, for use in `if` conditions and `static_assert`.
- **Custom alternative types.** A customization point that lets user
  types such as `Result<T, E>` work with `is<T>` and exhaustiveness
  checking, beyond `std::variant`.
- **`std::expected` patterns.** Value and error states, including
  `expected<T, T>`.
- **Positional aggregate destructuring in C++17.** Field-counting
  techniques limit this to small aggregates without base classes; the
  alternative is to wait for reflection mode.
- **Sequence patterns.** Matching the head and rest of a `vector` or
  `array`.
- **Conditional expression handlers.** `iif(cond, a, b)` would evaluate
  only the chosen expression. Calls and construction are scheduled for
  v0.9.7; conditional evaluation still needs a concrete use case.
- **More value types on the left of `>>`.** Accepting any
  equality-comparable type, not only arithmetic, enum and string
  types.
- **Values carrying domain metadata.** A customization point could
  let a bounded business type supply its own complete domain, avoiding
  a repeated `within` stage. An unsigned `mod<N>(x)` could return such
  a value with domain `[0, N)`. The name and ownership protocol are
  exploratory; signed remainder semantics and valid nonzero divisors
  must be explicit. Do not grow a general arithmetic DSL merely to
  remove one domain declaration.

---

## Released

Recent releases. Earlier versions are in the
[release notes](changelog/releases.md).

| Version | Highlights |
|---|---|
| [v0.9.5](changelog/v0.9.5.md) | `has<...>` is tuple-like for macro-free member names; `PTN_BIND` deprecated; bare `is<T>` / `alt<I>` reach static dispatch; handler classification no longer instantiates generic lambdas |
| [v0.9.4](changelog/v0.9.4.md) | `PTN_BIND` member-anchored guard names; operator sugar for `neg`, `any` and `all` |
| [v0.9.3](changelog/v0.9.3.md) | `pred(callable)` predicate pattern; `neg(p)` |
| [v0.9.2](changelog/v0.9.2.md) | `any(ps...)` and `all(ps...)` combinators |
| [v0.9.1](changelog/v0.9.1.md) | `val<V>` static literals, including C++20 floating-point values; canonical `has<...>` |

---

## Design Principles for New API

- Stateless compile-time patterns are variable templates (`is<T>`,
  `alt<I>`, `val<V>`), not function templates that return
  default-constructed objects.
- Function forms are reserved for patterns that take runtime arguments
  or subpatterns (`lit(value)`, `rng(lo, hi)`, `is<T>(p)`).
- Names are short, lowercase, and read naturally in the DSL.
- No new macros. `PTN_ON` is the only macro meant to stay.
- C++17 is the baseline. Newer language features are optional
  improvements and never change C++17 behavior.
- New patterns work on the general matching path first. Static
  dispatch is an optimization and must not change results.
- No heap allocation, RTTI or virtual dispatch in the matching path.

## Considered and Declined

Ideas that were evaluated and intentionally rejected, kept here so
they are not re-proposed without new motivation.

- **Matching class hierarchies with `dynamic_cast`**: conflicts with the
  no-RTTI principle. Match on a `std::variant` instead.
- **Macro syntax imitating a language `match` statement**: contradicts
  the no-new-macros principle.
- **`0 >> 1` without `lit`**: not possible. C++ only allows an
  overloaded operator when at least one operand has class or
  enumeration type, so two built-in operands always use the built-in
  shift.
- **Handlers that return from the enclosing function**: a handler is a
  callable, so `return`, `break` and `continue` inside it cannot leave
  the enclosing function or loop.
- **Detecting enumerators by scanning part of a wider type** (as
  magic_enum does with its default range of -128 to 127): an
  enumerator outside the scanned range would be missed, and a match
  that does not handle it would compile as complete.
- **Guard `operator!`**: intentionally not provided; rewrite the
  comparison instead (e.g. `!(x < y)` becomes `y <= x`).
- **Member-call placeholders** (`_.size() > 3`): not expressible —
  C++ has no `operator.` overloading, so `.size()` cannot become an
  expression node. Use `pred` with a lambda.
- **Chained comparisons** (`1 <= _ <= 10`): breaks predicate
  semantics and produces unreadable diagnostics. Use `rng(lo, hi)`
  with explicit range modes.
- **`PTN_BIND` `Type` validation**: resolved without reflection —
  names now expand to `member_t<&Type::name>`, so member checking
  happens at declaration time (shipped in v0.9.4).
