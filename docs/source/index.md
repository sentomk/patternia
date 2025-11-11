## patternia

----

patternia is a **header-only pattern matching DSL** for modern C++.

It provides a **declarative** and **composable** way to express conditional logic —  
bringing clarity and structure to branching code without macros or runtime cost.

```cpp
   using namespace ptn;

   auto res = match(cmd)
       .when(ci_value("start") >> "starting")
       .when(ci_value("stop")  >> "stopping")
       .otherwise("unknown");
```

----

### Why patternia?

- Write **expressive branching logic** without `if` or `switch`
- Build patterns with **value**, **relational**, and **predicate** combinators
- **Compile-time evaluation** — zero overhead, constexpr-capable
- Fully **header-only**, works with GCC, Clang, and MSVC

----

**Example: numeric range matching**

```cpp
   int x = 42;
   auto label = match(x)
       .when(lt(0) >> "negative")
       .when(between(1, 100) >> "in range")
       .otherwise("out of range");
```

----

### Getting Started

See the {doc}`usage` section for installation and examples.

For a five-minute tutorial, check out {doc}`Quick Start <guide/quickstart>`.

