<div align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" 
            srcset="https://wordpress-1316673449.cos.ap-beijing.myqcloud.com/img/banner-dark.svg">
    <source media="(prefers-color-scheme: light)" 
            srcset="https://wordpress-1316673449.cos.ap-beijing.myqcloud.com/img/banner-dark.svg">
    <img alt="Patternia logo"
         width="300"
         style="max-width: 90%; height: auto; margin-top: 10px; transform: translateX(-2px);">
  </picture>
</div>

<br>

<div align="center">

[![C++17+](https://img.shields.io/badge/C%2B%2B-17%2B-blue.svg?style=flat-square&logo=c%2B%2B&logoColor=white)](https://isocpp.org/)
[![Build](https://github.com/SentoMK/patternia/actions/workflows/ci.yml/badge.svg?branch=main&style=flat-square)](https://github.com/SentoMK/patternia/actions)
[![License](https://img.shields.io/github/license/SentoMK/patternia?style=flat-square)](LICENSE)
[![Version](https://img.shields.io/github/v/release/SentoMK/patternia?style=flat-square&color=orange)](https://github.com/SentoMK/patternia/releases)
[![Docs](https://img.shields.io/badge/docs-online-blue?style=flat-square)](https://sentomk.github.io/patternia)

</div>

REFACTOR

<br>

**A header-only, zero-overhead, compile-time pattern matching library for modern C++.**

---

## Overview

Patternia rethinks how branching logic is expressed in C++. Traditional control flow—if, else, switch, template specializations, and RTTI—spread decision-making across many syntactic forms. As projects grow, these mechanisms become fragmented, repetitive, and difficult to maintain, even though the underlying intent is often simple: match a subject against a set of well-defined conditions and execute the corresponding behavior.

Patternia introduces a unified Pattern System that elevates such conditions to first-class constructs. Instead of embedding logic directly into control structures, patterns describe what it means to match, while handlers define what to do when the match succeeds. This separation produces clearer logic, reusable rules, and a central dispatch mechanism that scales naturally from simple value checks to type-level and template-family matching.

The system is entirely zero-overhead. All pattern evaluation is resolved using static analysis, inline semantics, and ordinary C++ rules—without RTTI, virtual dispatch, or runtime penalties. As a result, Patternia provides the expressiveness of a high-level matching language while preserving the performance characteristics of hand-written branches.

By unifying value semantics, predicates, and type-level reasoning under a coherent model, Patternia offers a modern way to structure decision logic in C++: declarative where you want clarity, composable where you need complexity, and transparent when performance matters.

Examples, API references, patterns, and architectural documentation are now hosted on the project’s **GitHub Pages**:

**[Read the full documentation](https://sentomk.github.io/patternia)**

---

## To build and use

Ensure your development environment has the following tools installed and configured:

- [CMake (≥3.14)](https://cmake.org/)
- **Minimum**: C++17
- **Recommended compilers**: GCC >= 11, Clang >= 12, MSVC >= Visual Studio 2019
