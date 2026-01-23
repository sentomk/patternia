# Pattern Matching in Other Languages

## A Familiar Idea, Revisited in C++

If you have used pattern matching in languages like Rust, Scala, Haskell, or OCaml, you already understand the core promise of the technique. Pattern matching replaces imperative branching with a declarative description of cases. Instead of asking *how* to test conditions step by step, you state *which shapes of data are meaningful* and *what should happen when those shapes appear*.

Patternia deliberately builds on this shared intuition. A match expression evaluates cases in order, the first applicable case wins, and handlers operate under the assumption that their input has already been validated by the pattern. In this sense, Patternia behaves exactly as an experienced pattern-matching user would expect.

The differences begin to appear not at the conceptual level, but at the boundary between language features and library design.

---

## Expression-Oriented Matching Across Languages

In most modern pattern-matching systems, `match` is an expression. It produces a value rather than merely directing control flow. This is true in Rust:

```rust
let label = match x {
    0 => "zero",
    1 => "one",
    _ => "many",
};
```

Patternia adopts the same expression-oriented mindset. A match computes a result, and every case participates in producing that result:

```cpp
auto label = match(x, cases(
  lit(0) >> "zero",
  lit(1) >> "one",
  __     >> "many"
)).end();
```

The resemblance is intentional. For readers coming from expression-oriented languages, this form should feel immediately natural. What is different is what Patternia *does not* allow here. There are no bindings or guards. This mirrors the idea that value-only matching should remain simple and readable, rather than becoming a staging ground for complex logic.

---

## When Guards Enter the Picture

In many languages, guards are written inline with patterns. Rust, for example, allows conditions to be attached directly to a match arm:

```rust
match x {
    n if n > 0 && n < 10 => "small",
    _ => "other",
}
```

Patternia supports the same expressive power, but deliberately changes how it is introduced. Guards do not appear in the simple `cases()` form. Instead, the presence of a guard signals a transition to a more expressive matching mode:

```cpp
auto label = match(x)
  .when(bind()[_ > 0 && _ < 10] >> "small")
  .otherwise("other");
```

This separation is not accidental. In a library-based system, allowing arbitrary predicates everywhere would quickly blur the distinction between pattern matching and ad-hoc filtering. By requiring `bind()` before a guard can exist, Patternia enforces a clear rule: guards refine bound data; they do not replace matching itself.

For readers familiar with other languages, it is useful to think of this as making the “guarded arm” concept structurally explicit rather than syntactically implicit.

---

## Binding as a Visible Contract

One of the most striking differences for experienced users is how binding works. In many pattern-matching languages, variables introduced in patterns appear implicitly:

```rust
match value {
    Some(v) => process(v),
    None => {}
}
```

In Patternia, binding is always explicit:

```cpp
match(value)
  .when(bind() >> [](auto v) { process(v); })
  .otherwise([] {});
```

This is not an arbitrary restriction. In C++, implicit binding would interact poorly with overload resolution, template instantiation, and error diagnostics. By requiring `bind()`, Patternia makes the data contract between a pattern and its handler explicit and mechanically obvious.

Conceptually, this is still the same idea: the handler receives values that were extracted by the match. The difference is that Patternia chooses explicitness over inference, favoring long-term readability and predictability.

---

## Structural Matching Without Native Destructuring

Languages with native pattern matching often support destructuring as part of the syntax. Scala, for example, allows deep structural patterns directly in a case:

```scala
user match {
  case User(age, true) if age < 18 => "minor"
  case _ => "adult"
}
```

C++ does not provide a uniform destructuring facility for arbitrary types, so Patternia approaches structure differently. Instead of attempting to mirror language-level destructuring, it allows patterns to describe which parts of a value are relevant and then bind those parts explicitly.

```cpp
auto label = match(user)
  .when(
    bind(has<&User::age, &User::active>())
      [arg<0> < 18 && arg<1> == true] >>
    "minor"
  )
  .otherwise("adult");
```

The underlying intent is the same: separate *shape* from *behavior*. What changes is the surface syntax, shaped by the realities of C++’s type system rather than by an algebraic data model.

---

## Exhaustiveness and Fallbacks

Many pattern-matching languages emphasize exhaustiveness, often enforced by the compiler. While a library cannot replicate this fully, Patternia still encourages explicit design choices around fallbacks.

A wildcard pattern represents an intentional catch-all case, participating in ordering like any other case. A match-level fallback represents the absence of a match entirely. This distinction mirrors the conceptual difference, in other languages, between an explicit wildcard arm and an implicit runtime failure due to a non-exhaustive match.

The important point for experienced readers is that fallback behavior is never implicit. If something happens “by default,” it is written as a case or as an explicit fallback.

---

## Translating Ideas, Not Syntax

Patternia is not an attempt to recreate Rust, Scala, or Haskell syntax inside C++. It is an attempt to translate the *ideas* behind pattern matching into a form that remains idiomatic, predictable, and maintainable in a library setting.

If you already understand pattern matching elsewhere, learning Patternia is primarily about learning where boundaries are drawn. Simple matches stay simple. Binding is explicit. Guards refine rather than replace matching. Structure is described, not implicitly unpacked.

Once those boundaries are clear, the system should feel less like a foreign abstraction and more like a familiar concept expressed with C++’s own constraints in mind.
