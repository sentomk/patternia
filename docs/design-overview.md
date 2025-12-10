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

### 1.2 `.end()` (Statement-style Match Finalizer)

* `.end()` transforms the builder into **statement-style matching**:

  * The entire `match(...)` expression **does not return a value**.
  * Used only for side effects (printing, logging, state modification, etc.).
* Corresponds to the **statement form** in the standard proposal's `inspect`:

  * `pattern: statement;`
* Constraints:

  * All case handlers must return `void`.
  * For exhaustible types (enum, bool, variant-like), future **exhaustiveness checking** will be performed:

    * Compile-time diagnostics when not all enum values or alternatives are covered.

### 1.3 `.otherwise(...)` (Expression-style Match Finalizer)

* `.otherwise(fallback)` transforms the builder into **expression-style matching**:

  * The entire `match(...)` has a unified return value.
  * Similar to Rust's `let r = match x { ... };`
* Corresponds to the **expression form** in the standard proposal's `inspect`:

  * `pattern => expression,`
* Constraints:

  * All `.when(pattern >> handler)` handlers must return non-void and be type-unifiable.
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
| `.end()`       | Match Level    | Entire match "executed as statement" with no return |
| `.otherwise`    | Match Level    | Entire match "as expression" with final return value |

Therefore:

* `__` cannot replace `.otherwise()`:
  It doesn't terminate the builder, nor determine return type.
* `.otherwise()` cannot replace `__`:
  It cannot express "fallback for remaining constructors/fields in pattern tree".

---

## 3. `.end()` vs `.otherwise()`: Two Modes

### 3.1 `.end()` — Statement-style Matching

**Use Cases:**

* Only care about side effects, no return value needed.
* Want exhaustiveness checking for enum/variant-like types.

**Semantic Characteristics:**

* All handlers must return `void`.
* Return type is `void`.
* Exhaustible types will be checked for "complete coverage".

**Example (enum):**

```cpp
enum class Status { Pending, Running, Failed };

match(status)
    .when(lit(Status::Pending) >> [] { log("Pending"); })
    .when(lit(Status::Running) >> [] { log("Running"); })
    .when(__ >> [] { log("Other"); })  // pattern-level fallback
    .end();                            // match finalizer, no return value
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
auto res = match(x)
    .when(lit(1) >> "one")
    .when(lit(2) >> "two")
    .when(__ >> "other")    // __ is pattern-level fallback here
    .otherwise("panic");    // match-level fallback (defensive, usually never triggered)
```

* `__ >> "other"`: Represents "all other values".
* `.otherwise("panic")`: Ideally never used, but can serve as defensive fallback.

### 4.2 Remaining Branch in Enum/Variant-like Matching

For variant-like types (including future kind/alternative patterns):

```cpp
auto info = match(v)
    .when(type::is<int>         >> "int")
    .when(type::is<std::string> >> "string")
    .when(__                     >> "other-kind")
    .otherwise("panic"); // match-level defense, ideally never triggered
```

Here:

* `__` represents "all alternatives except int and string".
* `.otherwise("panic")` is the final match-level defense.

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

## 5. When Do `__` and `.otherwise()` "Appear Together"?

### 5.1 Typical Scenario: Expression Match + Pattern Fallback + Match Fallback

In complex modes:

* Pattern level might already use `__` for fallback matching all forms;
* But you can still set `.otherwise(...)` at match level as "defensive branch that should never trigger".

**Example:**

```cpp
auto label = match(v)
    .when(type::is<int>         >> "int")
    .when(type::is<std::string> >> "string")
    .when(__                     >> "other-kind") // pattern-level fallback
    .otherwise("unreachable");                    // match-level defense
```

Here `.otherwise("unreachable")` should ideally never trigger in normal logic, but:

* Can be used for debug/assertions;
* Semantically completely valid in the DSL.

### 5.2 Expression Matches Must Use `.otherwise()`

Whenever you write:

```cpp
auto res = match(subject)
    .when(...)
    ...
```

And want to assign the result to `res`, then **expression-style matches must be terminated with `.otherwise(...)`**.
Even if the pattern level is already covered by `__`.

The reason is:

* Builder needs a finalizer to determine "this is an expression-style match";
* C++ needs a clear return type, and `.otherwise(...)` is exactly this entry point.

### 5.3 When Does Match-level Fallback Trigger?

**The `.otherwise(handler)` is triggered in two scenarios:**

#### Scenario 1: No Pattern Matches
When none of the `.when()` cases match the subject value:

```cpp
auto result = match(42)
    .when(lit(1) >> "one")      // ❌ No match
    .when(lit(2) >> "two")      // ❌ No match  
    .when(__ >> "other")           // ❌ No match (if no wildcard case for this specific scenario)
    .otherwise("default");         // ✅ TRIGGERED: Returns "default"
```

#### Scenario 2: Explicit Match-level Defense
Even when patterns (including `__`) could theoretically match everything, `.otherwise()` still serves as a **defensive fallback**:

```cpp
auto result = match(42)
    .when(lit(1) >> "one")      // ❌ No match
    .when(lit(2) >> "two")      // ❌ No match
    .when(__ >> "other")           // ✅ PATTERN MATCHES: Returns "other"
    .otherwise("unreachable");      // ❌ NOT TRIGGERED: Pattern already matched
```

**Key Principles:**

1. **Pattern Priority**: Cases are evaluated in order. The first matching pattern wins.
2. **Wildcard Coverage**: `__` matches any value, so if placed before `.otherwise()`, it will typically match first.
3. **Defensive Purpose**: `.otherwise()` as "unreachable" provides runtime safety for logic errors or unexpected inputs.

#### Practical Example with Pattern vs Match Fallback:

```cpp
// This demonstrates the execution order
auto classify = std::string input;

auto result = match(classify)
    .when(lit_ci("error") >> "ERROR")           // Case 1: Specific match
    .when(lit_ci("warning") >> "WARNING")       // Case 2: Specific match  
    .when(__ >> "UNKNOWN")                      // Case 3: Pattern fallback
    .otherwise("INVALID INPUT");                 // Case 4: Match-level fallback (unlikely to trigger)

// For input "INFO":
// - Cases 1,2: No match
// - Case 3 (__): Matches → Returns "UNKNOWN"  
// - Case 4 (.otherwise): NOT triggered

// For input null/empty string (if it reaches match):
// - Cases 1,2,3: All could potentially handle this
// - Depending on implementation, might reach .otherwise() as defense
```

**Summary:**
- **Pattern-level fallback (`__`)**: Only affects case matching process at pattern level
- **Match-level fallback (`.otherwise()`)**: Determines final return value of match expression
- **When `__` can match all pattern forms**: `.otherwise()` serves only as defensive fallback and typically won't trigger

---

## 6. When to Use `.end()`, When to Use `.otherwise()`?

Use this decision table to directly guide user usage.

### 6.1 Decision Table

| Requirement                                   | Recommended Usage       |
| ------------------------------------------- | --------------------- |
| Only execute side effects, no match return value | Use `.end()`          |
| Need to get a value from match (like Rust)     | Use `.otherwise(...)` |
| Want exhaustiveness checking for enum/variant     | Use `.end()`          |
| Express "all remaining matching forms"           | Use `__` in pattern   |
| Express "entire match fallback return value"      | Use `.otherwise(...)` |

### 6.2 Typical Patterns

1. **Statement-style with fallback:**

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
* `.end()` creates an empty fallback handler for statement-style execution.
* `.otherwise()` uses the provided handler as the final fallback for expression-style execution.

### 7.2 Type Safety Guarantees

* **Statement-style (`.end()`)**: All handlers must return `void`
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
   * Non-void handler under `.end()` → Clear error.
   * Handler return type inconsistency under `.otherwise()` → Clear type diagnostics.
   * Wildcard + `.otherwise()` redundancy → Hint "potentially redundant patterns".

---

This comprehensive design ensures that Patternia provides clear, predictable, and user-friendly pattern matching semantics while maintaining type safety and performance guarantees.
