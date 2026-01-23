# Patternia Matching Semantics Overview

(`__`, `.end()`, `.otherwise()` and Destructuring Patterns)

## 0. Core Objectives

Patternia's DSL is designed to satisfy three key principles:

1. **Semantic Clarity**:
   - Clear separation between "pattern-level" and "match-level" responsibilities.
2. **Predictable Behavior**:
   - Consistent with Rust and the C++ standard proposal P1371R1 `inspect` mechanism in spirit.
3. **User-Friendly**:
   - Minimal API coverage for both "statement-style matching" and "expression-style matching" requirements.

Around these three principles, the entire system can be abstracted into three levels:

1. **Pattern Level**: `lit(...)`, `type::is<T>`, destructuring, `bind(...)`, `__`, etc.
2. **Case Level**: `.when(pattern >> handler)`.
3. **Match Finalizer Level**: `.end()` / `.otherwise(...)`.

All discussions below revolve around these three levels.

---

## 1. Formal Definitions of Core Concepts

### 1.1 `__` (Wildcard Pattern)

* `__` is a **pattern** used to match "any value".
* It exists only at the **Pattern Level** and does not control the entire match lifecycle.
* It plays different roles in different contexts:

  * In **literal matching**: Serves as a "fallback case".
  * In **enum/variant-like matching**: Serves as a "fallback for remaining constructors".
  * In **struct destructuring**:
    Used for "field-level ignoring", similar to Rust's `_` / `..`.

### 1.2 `.end()` (Match Finalizer for `__`)

* `.end()` finalizes a match that uses the `__` pattern fallback:

  * The entire `match(...)` expression may return a value or be `void`.
  * Used for exhaustive-style matches where `__` is a regular case.
* Corresponds to the **statement form** in the standard proposal's `inspect`
  when handlers are `void`-returning.
* Constraints:

  * All case handlers must return compatible types (or all `void`).
  * For exhaustible types (enum, bool, variant-like), future **exhaustiveness checking** will be performed:

    * Compile-time diagnostics when not all enum values or alternatives are covered.

### 1.3 `.otherwise(...)` (Expression-style Match Finalizer)

* `.otherwise(fallback)` transforms the builder into **expression-style matching**:

  * The entire `match(...)` has a unified return value.
  * Similar to Rust's `let r = match x { ... };`
* Corresponds to the **expression form** in the standard proposal's `inspect`:

  * `pattern => expression,`
* Constraints:

  * All `.when(pattern >> handler)` handlers must return compatible types (or all `void`).
  * `.otherwise(...)` provides the final fallback value:

    * Used when no branches match.
  * `.otherwise(...)` terminates the builder and determines the return type.

---

## 2. Semantic Levels: Pattern vs Match Finalizer

Always remember:

> **`__` is a Pattern-level object;
> `.end()` and `.otherwise()` are Match Finalizer-level objects.**

They solve completely different problems:

| Role           | Level          | Problem Solved                              |
| --------------- | -------------- | ----------------------------------------- |
| `__`           | Pattern Level  | How current case matches "any/remaining forms" |
| `.end()`       | Match Level    | Finalizes a match that uses `__` as a case |
| `.otherwise`    | Match Level    | Entire match "as expression" with final return value |

Therefore:

* `__` cannot replace `.otherwise()`:
  It doesn't terminate the builder, nor determine return type.
* `.otherwise()` cannot replace `__`:
  It cannot express "fallback for remaining constructors/fields in pattern tree".

---

## 3. `.end()` vs `.otherwise()`: Two Modes

### 3.1 `.end()` — `__`-Finalized Matching

**Use Cases:**

* Want to use `__` as a regular case.
* Want exhaustiveness checking for enum/variant-like types.

**Semantic Characteristics:**

* All handlers must return compatible types (or all `void`).
* Return type is deduced from the cases.
* Exhaustible types will be checked for "complete coverage".

**Example (enum):**

```cpp
enum class Status { Pending, Running, Failed };

match(status)
    .when(lit(Status::Pending) >> [] { log("Pending"); })
    .when(lit(Status::Running) >> [] { log("Running"); })
    .when(__ >> [] { log("Other"); })  // pattern-level fallback
    .end();                            // match finalizer for `__`
```

**Rust Analogy:**

```rust
match status {
    Status::Pending => log("Pending"),
    Status::Running => log("Running"),
    _               => log("Other"),
};
```

### 3.2 `.otherwise(...)` — Expression-style Matching

**Use Cases:**

* Need to **compute a value** from match:
  * Generate labels for enum/variant;
  * Calculate results, map strings, etc.

**Semantic Characteristics:**

* All handlers return the same type (or commonly convertible).
* `.otherwise(fallback)` provides the final value.
* The entire `match(...)` expression has a concrete type.

**Example (Rust counterpart):**

```cpp
int x = 2;

auto res = match(x)
    .when(lit(1) >> "one")
    .when(lit(2) >> "two")
    .otherwise("Other");

std::cout << res;
```

**Corresponding Rust:**

```rust
let x = 2;
let res = match x {
    1 => "one",
    2 => "two",
    _ => "Other",
};
println!("{}", res);
```

---

## 4. `__` Roles in Different Scenarios

### 4.1 As "Case Fallback" Wildcard

Simplest literal scenario:

```cpp
match(x)
    .when(lit(1) >> [] { /* one */ })
    .when(lit(2) >> [] { /* two */ })
    .when(__ >> [] { /* other */ })    // __ is pattern-level fallback here
    .end();
```

* `__ >> ...`: Represents "all other values".

### 4.2 Remaining Branch in Enum/Variant-like Matching

For variant-like types (including future kind/alternative patterns):

```cpp
match(v)
    .when(type::is<int>()         >> [] { /* int */ })
    .when(type::is<std::string>() >> [] { /* string */ })
    .when(__                       >> [] { /* other-kind */ })
    .end();
```

Here:

* `__` represents "all alternatives except int and string".

### 4.3 Field Wildcard in Struct Destructuring

**Corresponding Rust:**

```rust
match p {
    Point { x, .. } => println!("{}", x),
}
```

**Patternia ideal DSL example:**

```cpp
struct Point { int x, y, z; };

match(p)
    .when(Point{ bind(x), __, __ } >> [&] {
        std::cout << x;
    })
    .end();
```

**Meaning:**

* Matches object of type `Point`.
* Only binds first field `x`.
* Other fields (`y`, `z`) are ignored with `__`.

`__` here is **field-level wildcard**, not a fallback case.
Future named field versions could also be extended:

```cpp
.when(Point{ .x = bind(x), .y = __, .z = __ } >> ...)
```

---

## 5. `__` and `.otherwise()` Are Mutually Exclusive

`__` is a pattern-level fallback, while `.otherwise()` is a match-level fallback.
Patternia rejects `.otherwise()` if a wildcard case is present. Use either:

* `__` with `.end()` for pattern-level fallback, or
* `.otherwise(...)` without `__` for match-level fallback.

---

## 6. When to Use `.end()`, When to Use `.otherwise()`?

Use this decision table to directly guide user usage.

### 6.1 Decision Table

| Requirement                                   | Recommended Usage       |
| ------------------------------------------- | --------------------- |
| Use `__` as a case fallback                      | Use `.end()`          |
| Need a value without `__`                       | Use `.otherwise(...)` |
| Want exhaustiveness checking for enum/variant     | Use `.end()`          |
| Express "all remaining matching forms"           | Use `__` in pattern   |
| Express "entire match fallback return value"      | Use `.otherwise(...)` |

### 6.2 Typical Patterns

1. **`__`-finalized with fallback:**

   ```cpp
   match(s)
       .when(lit(Status::Pending) >> [] { ... })
       .when(lit(Status::Running) >> [] { ... })
       .when(__ >> [] { ... }) // fallback
       .end();
   ```

2. **Expression-style, Rust counterpart:**

   ```cpp
   auto res = match(x)
       .when(lit(1) >> "one")
       .when(lit(2) >> "two")
       .otherwise("Other");
   ```

3. **Struct destructuring + field wildcard:**

   ```cpp
   match(p)
       .when(Point{ bind(x), __, __ } >> [&] { return x; })
       .otherwise(0);
   ```

4. **Variant-like + type matching + pattern fallback:**

   ```cpp
   auto info = match(v)
       .when(type::is<int>         >> "int")
       .when(type::is<std::string> >> "string")
       .when(__                     >> "other-kind")
       .otherwise("panic");
   ```

---

## 7. Implementation Details

### 7.1 Builder Execution Flow

The builder pattern ensures that matches are only executed when properly terminated:

```cpp
// Without .end() or .otherwise():
match(value).when(lit(42) >> [] { /* handler */ });
// Result: Nothing is executed - builder remains unterminated

// With .end():
match(value).when(lit(42) >> [] { /* handler */ }).end();
// Result: match_impl::eval(subject_, cases_, std::move(dummy_fallback));

// With .otherwise():
match(value).when(lit(42) >> [] { /* handler */ }).otherwise(fallback);
// Result: match_impl::eval(subject_, cases_, std::move(final_handler));
```

**Key Points:**

* Without `.end()` or `.otherwise()`, the builder never triggers execution.
* `.end()` creates an empty fallback handler for `__`-finalized execution.
* `.otherwise()` uses the provided handler as the final fallback for expression-style execution.

### 7.2 Type Safety Guarantees

* **`.end()`**: All handlers must return compatible types (or all `void`)
* **Expression-style (`.otherwise()`)**: All handlers must return compatible types
* **Pattern matching**: Type-safe heterogeneous comparisons
* **Binding**: Compile-time type deduction for bound values

---

## 8. Future Roadmap

To make these semantics truly user-friendly, Patternia's future plans include:

1. **Exhaustiveness Checking for `.end()`**:
   * Enum/bool: Static checking for complete enum value coverage.
   * Variant-like: Check for complete alternative coverage.
   * Provide "missing case" and "useless case" hints.

2. **Formal Struct Destructuring DSL**:
   * Positional: `Point{ bind(x), __, __ }`
   * Named: `Point{ .x = bind(x), .y = __, .z = __ }`
   * Support guard/bind/nested patterns.

3. **Improved Error Messages**:
   * Incompatible handler return types under `.end()` → Clear error.
   * Handler return type inconsistency under `.otherwise()` → Clear type diagnostics.
   * Wildcard + `.otherwise()` redundancy → Hint "potentially redundant patterns".

---

This comprehensive design ensures that Patternia provides clear, predictable, and user-friendly pattern matching semantics while maintaining type safety and performance guarantees.
