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

<br>

**A header-only, zero-overhead pattern matching library for modern C++ that transforms branching logic into expressive, maintainable code.**

---

## ✨ Key Features
- **Zero Runtime Overhead**: Compile-time optimization with no RTTI or virtual dispatch
- **Intuitive Syntax**: Declarative pattern matching with clean DSL operators (`&&`, `||`, `!`)
- **Type Safety**: Compile-time guarantees with heterogeneous comparisons
- **Rich Patterns**: Value matching, type checking, ranges, predicates, and template introspection
- **Modern C++**: C++17/20 compatible with constexpr support
- **Header-Only**: Drop-in integration with no build dependencies
  
---

## 🚀 Quick Start

```cpp
#include "ptn/patternia.hpp"
using namespace ptn;
int main() {
    int x = 42;
    auto result = match(x)
        .when(lit(42) >> "answer")
        .when(between(1, 10) >> "small")
        .otherwise("other");
}
```
---

## 🔧 Installation

---

## 💡 Example Use Cases

### Complex Pattern Composition

```cpp
auto positive_even = pred([](int x) { return x > 0; }) && 
                     pred([](int x) { return x % 2 == 0; });
match(42).when(positive_even >> "valid");
```

---

## 📚 Documentation


---

## 🛠️ Build Requirements

- **Compilers**: GCC ≥11, Clang ≥12, MSVC ≥2019
- **CMake**: ≥3.14 (for examples/tests)
- **C++ Standard**: C++17 or newer

---

## 🤝 Contributing
  
---

<div align="center">
  <sub>Built with ❤️ for modern C++ development</sub>
</div> 
