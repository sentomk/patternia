# `ptn::match()` and the Match Builder

The `ptn::match()` function is the entry point to the Patternia DSL. It accepts a **Subject Value** to be matched against, and returns a **Match Builder** object, which allows the user to chain calls for adding patterns and handlers.

---

## 1. Entry Function: `ptn::match`

### Signature

This function is templated to accept any subject value, preserving its value category (e.g., it stores a copy for an lvalue, or moves the value for an rvalue).

```cpp
namespace ptn {

  template <typename T>
  constexpr auto match(T &&value) noexcept;

} // namespace ptn
```

### Description

Constructs an instance of the **`match_builder`** containing a copy or moved version of the `value`. This function signifies the beginning of a pattern matching sequence.

### Template Parameters

| Name | Description                                        |
| :--- | :------------------------------------------------- |
| `T`  | The type of the Subject Value passed to `match()`. |

### Parameters

| Name    | Type   | Description                                           |
| :------ | :----- | :---------------------------------------------------- |
| `value` | `T &&` | The value to be matched against the defined patterns. |

### Returns

**Type:** `ptn::core::match_builder<std::decay_t<T>, std::tuple<>>`

Returns an initialized match builder object, with the decayed type of $T$ as its subject type. The initial case tuple is empty.

### Example

```cpp
#include <ptn/patternia.hpp>
using namespace ptn;

// Matches an int (lvalue copy)
int x = 10;
auto builder_int = match(x);

// Matches an rvalue string (move semantics)
auto builder_str = match(std::string("hello"));
```

---

## 2\. The Match Builder: `ptn::core::match_builder`

The `match_builder` is internally an **immutable** class. Each call to `.when()` returns a **new** builder instance with a different type, which statically records the newly added pattern-handler pair.

### Signature

```cpp
namespace ptn::core {

  template <typename TV, typename... Cases>
  class match_builder { /* ... */ };

} // namespace ptn::core
```

### Member Functions

#### A. `.when()`

Appends a new pattern-handler pair (a Case) to the matching sequence.

##### Signature

```cpp
template <typename Pattern, typename Handler>
constexpr auto when(dsl::case_expr<Pattern, Handler> &&e) &&;
```

##### Description

Appends a `case_expr` (created via the `Pattern >> Handler` expression) to the compile-time list of cases. This operation is **chainable** and returns a new `match_builder` object that incorporates the new case.

##### Returns

**Type:** `ptn::core::match_builder<TV, Cases..., NewCase>`

Returns a new `match_builder` instance, which includes all previous cases plus the newly added case.

---

#### B. `.otherwise()`

Finalizes the matching sequence, triggering the evaluation and returning the result.

##### Signature

```cpp
template <typename H>
constexpr auto otherwise(H &&fallback) &&;
```

##### Description

Terminates the matching chain and performs evaluation:

1.  All patterns defined via `.when()` are checked sequentially.
2.  If the first matching pattern is found, its corresponding `Handler` is executed, and the result is returned.
3.  If no patterns match, the `fallback` handler is executed instead.
4.  **Note:** The final return type (`R`) is computed as the **`std::common_type_t`** of the return types of all handlers (including the `fallback`).

##### Parameters

| Name       | Type   | Description                                                                   |
| :--------- | :----- | :---------------------------------------------------------------------------- |
| `fallback` | `H &&` | The handler (function, lambda, or value) to be executed if no patterns match. |

##### Returns

**Type:** `R` (The common type of all handlers' return values)

Returns the result of the first successful pattern's handler, or the result of the `fallback` handler.
