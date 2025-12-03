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

Patternia is **header-only**, so installation is lightweight and dependency-free.

### Recommended: CMake FetchContent

Add this to your project’s **top-level** `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.14)
project(your_project_name LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    patternia
    GIT_REPOSITORY https://github.com/SentoMK/patternia.git
    GIT_TAG main
)
FetchContent_MakeAvailable(patternia)

add_executable(your_project_name main.cpp)
target_link_libraries(your_project_name PRIVATE patternia::patternia)
```

This automatically fetches Patternia at configure time and sets up the imported target.

### More Installation Options

Source installation, header-only drop-in, Conan/vcpkg integration and versioning instructions can be found in the Online Docs:

Documentation → [Installation Guide](https://sentomk.github.io/patternia/guide/installation/)

---

### Supported Platforms

| OS      | Compilers            | Status          |
| ------- | -------------------- | --------------- |
| Linux   | GCC ≥11, Clang ≥12   | **Fully Supported** |
| Windows | MSVC ≥2019, Clang-CL | **Fully Supported** |
| macOS   | AppleClang ≥14       | **Fully Supported** |

---

## 🤝 Contributing

Contributions are welcome. Whether it is **bug reports**, **feature proposals**, or **pull requests**, your help is appreciated.

### Before Submitting Changes

1. **Discuss First**
   Open an Issue or a Discussion when proposing new features or behavior changes.
2. **Code Style**
   Follow modern C++ idioms and ensure:

   * C++17+ compatible
   * No RTTI or virtual dispatch introduced
   * Zero-overhead principles preserved
3. **Tests Required**
   Add or update unit tests under `tests/` for any logic changes.
4. **Comment Style**
   Follow **Google C++ Style Guide** for documentation comments.
5. **Commit Style**
   Use conventional commit messages:

   ```
   feat: add predicate pattern for ranges
   fix: correct match_result type inference
   refactor: reorganize dsl ops
   ```

### Development Setup

```bash
git clone https://github.com/SentoMK/patternia.git
cd patternia
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Run tests:

```bash
ctest --test-dir build
```

---

<div align="center">
  <sub>Built with ❤️ for modern C++ development</sub>
</div> 
