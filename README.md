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
- **Intuitive Syntax**: Declarative pattern matching with clean DSL operator (`>>`)
- **Type Safety**: Compile-time guarantees with heterogeneous comparisons
- **Rich Patterns**: Literal matching, case-insensitive string matching
- **Modern C++**: C++17+ compatible with constexpr support
- **Header-Only**: Drop-in integration with no build dependencies
  
---

## 🚀 Quick Start

### Basic Example: Command Processing

Transform scattered if-else chains into clean, declarative pattern matching:

```cpp
#include <ptn/patternia.hpp>
#include <iostream>
#include <string>

using namespace ptn;

std::string process_command(const std::string& cmd) {
    return match(cmd)
        .when(lit("start") >> "System starting...")
        .when(lit("stop") >> "System stopping...")
        .when(lit("restart") >> "System restarting...")
        .when(lit_ci("help") >> "Available commands: start, stop, restart, help")
        .otherwise("Unknown command");
}

int main() {
    std::cout << process_command("START") << "\n";  // Case-insensitive match
    std::cout << process_command("stop") << "\n";
    std::cout << process_command("invalid") << "\n";
}
```

**Output:**
```
Available commands: start, stop, restart, help
System stopping...
Unknown command
```

### What this demonstrates

| Benefit                             | Description                                                                       |
| ----------------------------------- | --------------------------------------------------------------------------------- |
| **Logic & behavior separation**     | Commands and responses are cleanly separated                                               |
| **Readable business intent**        | No mixed branching or hidden side effects                                         |
| **Case-insensitive matching**    | Built-in support for common string matching needs                                    |
| **Zero runtime overhead**           | All decisions are resolved by the compiler                                        |

This shows how Patternia transforms traditional control flow into expressive, safe, and maintainable logic — ideal for command processing, protocol handling, or any domain where input mapping is required.

---

### Why not `switch` or `if-else`?

Traditional approaches:

```cpp
if (cmd == "start") { ... }
else if (cmd == "stop") { ... }
else if (cmd == "restart") { ... }
else if (cmd == "help" || cmd == "HELP" || cmd == "Help") { ... }
```

or:

```cpp
switch(hash(cmd)) { ... }
```

These:

* tightly couple **condition** and **behavior**
* become error-prone when branching grows
* require repetitive code for case-insensitive matching
* are hard to extend without modifying existing logic

Patternia solves all of these, while generating equally efficient (or better) machine code.

---

## 🔧 Installation

Patternia is **header-only**, so installation is lightweight and dependency-free.

### Recommended: CMake FetchContent

Add this to your project's **top-level** `CMakeLists.txt`:

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

Source installation, header-only drop-in, Conan/vcpkg integration and versioning instructions can be found in Online Docs:

Documentation → [Installation Guide](https://sentomk.github.io/patternia/guide/installation/)

---

### Supported Platforms

| OS      | Compilers            | Status          |
| ------- | -------------------- | --------------- |
| Linux   | GCC ≥11, Clang ≥12   | **Fully Supported** |
| Windows | MSVC ≥2019, Clang-CL | **Fully Supported** |
| macOS   | AppleClang ≥14       | **Fully Supported** |

---

## 📚 Current Features

### Core Matching
- `match(value)` - Type-deduced matching entry point
- `match<T>(value)` - Explicit type conversion matching
- `.when(pattern >> handler)` - Add matching branches
- `.otherwise(handler)` - Default fallback case

### Pattern Types
- `lit(value)` - Exact value matching for any comparable type
- `lit_ci(value)` - Case-insensitive ASCII string matching

### Supported Value Types
- Arithmetic types (int, double, float, etc.)
- Enum types
- String types (std::string, std::string_view, const char*)
- User-defined types (must support operator==)

### Handler Types
- Value handlers (return fixed value)
- Function handlers (process matched value)

---

## 🗺️ Roadmap

Patternia is actively being developed. Planned features include:

- [ ] Predicate patterns (`pred(lambda)`)
- [ ] Relational patterns (`lt`, `gt`, `between`, etc.)
- [ ] Logical operators (`&&`, `||`, `!`)
- [ ] Type patterns (`type::is<T>`, `type::in<Ts...>`)
- [ ] Template pattern matching
- [ ] Structural pattern matching

Check the [Roadmap](https://sentomk.github.io/patternia/design/roadmap/) for detailed planning.

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
   feat: add case-insensitive string matching
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
