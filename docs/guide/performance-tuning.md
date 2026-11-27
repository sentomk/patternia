# Performance Tuning

A Patternia match is meant to cost what the same branches cost when
written by hand. For the common shapes it does: a dense literal match
compiles to the code of a `switch`, and a variant match to the code of
a `switch` on `index()`. This page explains which matches take those
paths, what keeps them there, and how to check.

## How a match is evaluated

The case list is planned at compile time. The plan depends only on the
subject's type and on the patterns, never on the values:

| Case list | Plan | Cost |
|---|---|---|
| `val<N>` literals on an integral or enum subject (scoped `enum class` included), keys close together, an optional `_` last | Dense table: the subject's offset from the smallest key indexes one `switch` | One compare, then a jump or lookup table; independent of the number of cases |
| `lit(v)` literals on an integral or enum subject, an optional `_` last | The first case whose value equals the subject is found by comparisons that each yield a constant, then run through the same `switch` | Like a hand-written `switch`: compilers build a table when the keys allow it |
| A `std::variant` subject with `is<T>`, `alt<I>`, `$(is<T>)`, `$(alt<I>)` cases, guarded or not, among any others | A `switch` on `index()`; each alternative runs only the cases that can match it, in source order | One switch, then the cases of one alternative; inside it, cases that only test the alternative do not test it again |
| Anything else: guards on other patterns, structural patterns, predicates, strings | Source order | One test per case until a case matches, like an `if` chain |

All plans keep first-match-wins semantics: a case runs only if no
earlier case matches, and guards are called in the same order and the
same number of times as in source order.

A dense table is used when the case list has at most 256 cases and the
keys span at most 512 values and at most 4 values per literal (8 per
literal from 4 literals up). `val<1>` to `val<64>` qualifies;
`val<0>, val<1000>` does not and is compared in order.

## Writing matches that stay keyed

- Use `val<N>` for compile-time constants. `val<N>` keys are known to
  the planner and can form a dense table; `lit(v)` values are only
  known at run time and are compared first.
- Keep the fallback last. `_` (or `$`) after literal cases is the
  table's default slot.
- Mixing a guard or a non-literal pattern into a literal list moves the
  whole list to source order, and so does mixing `val<N>` with `lit(v)`
  in one list. Put the literal cases in their own match if the rest
  needs guards.
- `val<N>` dense tables apply to integral and enum subjects, scoped
  `enum class` included.
- For variants, any mix of cases is fine: the switch on `index()`
  happens first, and only the cases that can match the active
  alternative are tested.
- Expression handlers (`$ >> _ * 2`) and guards (`$[_ > 0]`) compile to
  the same code as the equivalent lambdas.

## Borrowed and owned subjects

An lvalue subject is borrowed: `match(x)` reads `x` through a
reference. An rvalue is owned: `match(std::move(x))` or
`match(make_x())` moves the value into the match context first.

Owned subjects use the same plans as borrowed ones, with one exception:
a variant with four or fewer alternatives is evaluated in source order,
because compilers already turn those few index tests into tight code
and some (Apple Clang 17) compiled the switch into a tree of compares.
From five alternatives on, an owned variant match switches on
`index()` like a borrowed one.

The move into the context is the only extra cost. Moving a
`std::string` or a `std::unique_ptr` is cheap; matching an rvalue that
is expensive to move costs that move.

## Caching case packs

`match(x) | on(...)` builds its case pack on every call. When the
patterns and handlers are integers, captureless lambdas or
expressions, the compiler folds the construction away and nothing
remains. Caching matters when building the cases does work: a handler
such as `>> std::string("unknown")` constructs a string on every call,
and a `lit(std::string(...))` pattern stores one.

- `PTN_ON(...)` caches the case pack in a function-local static on the
  first call. Lambdas inside it must not capture.
- `static_on(factory)` does the same with an explicit factory, so the
  cached pack can be returned from a function and used elsewhere.
- A case pack you cache yourself should be `const`:
  `static const auto cases = on(...);`, or `constexpr` when the cases
  allow it.

`PTN_ON` and `static_on` cache the cases as `const` when no handler
keeps state, so the compiler treats them as constants, and a match
through `PTN_ON` compiles to the same code as `on(...)`. A handler
with state, such as a function object with a counter, keeps a mutable
cache, and the cases are read from memory. GCC turns a dispatch on a
non-`const` static into a jump table instead of a lookup table, which
made a 32-alternative variant match about three times slower in the
benchmarks (1.32 against 0.41 ns).

```cpp
int classify(int x) {
  return match(x) | PTN_ON(
    val<0> >> 0, val<1> >> 1, val<2> >> 2,
    _ >> -1
  );
}

constexpr auto parity = on(val<0> >> "even", val<1> >> "odd", _ >> "?");
const char *p = match(n % 2) | parity;
```

## Value handlers

A value handler (`pattern >> value`) stores the value in the case pack
and returns a copy of it each time the case is selected. Caching the
pack with `PTN_ON` saves building the value, not this copy. For
integers, enums and string literals the copy is free, and
literal-table plans read the stored value directly. For a
`std::string`, or a `std::variant` whose alternatives include one,
every match pays for the copy: a copy reads the stored value from
memory, while constructing from a literal lets the compiler fold it
into the call site.

Store a cheap value and convert it when the case is selected, with
[`| to<T>`](../api.md#to) or an explicit result type
[`on<R>`](../api.md#on-result), or use a lambda:

```cpp
using Outcome = std::variant<Accepted, Ignored>;

// Copies the stored Outcome on every fallback.
match(e) | on($(is<Order>) >> accept, _ >> Outcome{Ignored{}});

// Each of these stores the empty Ignored or a string literal and
// constructs the result only when the case is selected.
match(e) | on($(is<Order>) >> accept, _ >> Ignored{} | to<Outcome>);
match(e) | on<Outcome>($(is<Order>) >> accept, _ >> Ignored{});
match(x) | on<std::string>(lit(0) >> "zero", _ >> "a long default text");
```

In the measurements for
[issue #132](https://github.com/sentomk/patternia/issues/132), a
fallback returning a stored `Outcome` took 4.0 ns per call with Apple
Clang 17 and 2.6 ns with GCC 13; `_ >> Ignored{} | to<Outcome>` took
1.4 and 2.1 ns, the same as a hand-written `std::get_if`. A stored
`std::string` longer than the small-string buffer took about 37 ns
against 17 to 19 ns when built in a lambda.

## Compilers and build settings

- Build with optimization (`-O2` or `-O3`, `/O2`). The plans above
  describe optimized code; without optimization, every step of a
  match is a separate function call.
- The dispatch path is marked always-inline (`__attribute__((always_inline))`
  on GCC and Clang, `__forceinline` on MSVC). This keeps the case pack
  out of memory regardless of the inliner's size estimates, which on
  GCC also depend on the order of functions in the translation unit.
- Debug information: GCC writes the name of every template
  instantiation, with its arguments, into the debug information.
  Patternia keeps the case pack's type out of per-case names, so a
  64-case dense literal match costs about 1.3 MB of debug information
  with GCC 13 (`-O2 -g`, without `pubnames` as on Linux) and 0.3 MB
  with Clang. On macOS, GCC also writes
  `pubnames` by default; `-gno-pubnames` removes most of the rest.
- MSVC has no portable way to force a lambda inline; the dispatch
  functions themselves are forced, and MSVC's inliner decides for the
  continuations.

## Checking the generated code

`scripts/check_codegen.py` compiles a function containing one match
and checks the machine code: no calls, no stack use, at most a given
number of conditional branches or instructions, or no more
instructions than a reference function written by hand in the same
file. The probes in `tests/codegen/` run in CTest with the compiler of
the build:

```bash
python3 scripts/check_codegen.py --compiler g++ --include include \
  --source tests/codegen/literal_dense_64.cpp \
  --reference ptn_codegen_reference
```

To check a match of your own, put it in an `extern "C" int
ptn_codegen_probe(...)` function, optionally with the hand-written
equivalent as `ptn_codegen_reference`, and run the script on it. See
[`scripts/README.md`](https://github.com/sentomk/patternia/blob/main/scripts/README.md).

## Benchmarks

The benchmark report is published on the
[`bench-charts` branch](https://github.com/sentomk/patternia/blob/bench-charts/latest.md)
after every push to `main` (GCC 13 on a GitHub-hosted x86-64 runner).
It normalizes each scenario to the fastest implementation, and
Patternia is not the fastest in every scenario. Pull requests get a
base-against-PR comparison with a noise estimate. See
[Performance Notes](../performance/index.md) for per-release
measurements and methods.
