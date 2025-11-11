Introduction
============

Patternia is a **header-only pattern matching DSL** for modern C++.  
It allows writing expressive branching logic in a functional, composable way — without macros or heavy syntax.

.. code-block:: cpp

   using namespace ptn;

   auto result = match(cmd)
       .when(ci_value("start") >> "starting")
       .when(ci_value("stop")  >> "stopping")
       .otherwise("unknown");

Motivation
----------

Traditional `if`/`switch` statements are limited.  
Patternia introduces *declarative* pattern matching inspired by functional languages like Scala, Rust, and Haskell — but implemented in pure C++ templates.

Design Philosophy
-----------------

- **Declarative:** Focus on *what* to match, not *how*.  
- **Composable:** Patterns combine with `&&`, `||`, and custom predicates.  
- **Modern:** Designed for C++17–26 with constexpr and concepts support.
