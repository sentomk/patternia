# Pattern System Architecture

Patternia is built around a single architectural idea:  
**matching should be a first-class, composable, zero-overhead operation in C++.**

Most C++ projects rely heavily on hand-written branching logic—nested `if` statements, large `switch` blocks, duplicated comparisons, and one-off helper functions.  
These mechanisms are powerful but structurally limited:

- Logic becomes scattered across functions.  
- Conditions cannot be abstracted or composed easily.  
- Type-dependent paths require RTTI or explicit `dynamic_cast`.  
- Value-dependent paths become deeply imperative and repetitive.  
- Maintenance requires editing many disconnected branches.

Patternia introduces a dedicated **Pattern System** to unify these forms of branching under a single semantic model.

At its core, a **Pattern** is an object that describes a condition, and the system determines how to evaluate that condition safely and efficiently.  
All patterns—whether they describe a value, a predicate, or a type—share the same conceptual role:

1. **Describe a match rule.**  
   A pattern encapsulates the logic for deciding whether a subject satisfies a condition.

2. **Compose into larger rules.**  
   Patterns form an algebra: they can be combined into richer logic without changing control flow.

3. **Participate in the matching pipeline.**  
   The system evaluates patterns in sequence and invokes handlers for the first matching rule.

The Pattern System is not an auxiliary feature.  
It is the architectural foundation on which the entire library—value dispatch, type dispatch, and later variant dispatch—is built.

This chapter examines the architectural model of the Pattern System and clarifies how different pattern categories cooperate to define matching behavior. The discussion begins with value patterns and type patterns as foundational components.

- **[Value Patterns](patterns/value.md)**  
- **[Type Patterns](patterns/type.md)**

Their APIs appear elsewhere; here we focus on their **design purpose, abstraction model, and role within the match pipeline**.
