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
- **Rich Patterns**: Literal matching, binding patterns, case-insensitive string matching
- **Modern C++**: C++17+ compatible with constexpr support
- **Header-Only**: Drop-in integration with no build dependencies
  
---

## 🚀 Quick Start

### Basic Example: Wildcard and Enum Matching

Demonstrate pattern matching with wildcard, literal, and binding patterns:

```cpp
#include <ptn/patternia.hpp>
#include <iostream>

using namespace ptn;

enum Status {
  Pending,
  Running,
  Completed,
  Failed
};

int main() {
  Status s = Status::Running;

  auto result =
      match(s)
          .when(lit(Status::Pending) >> [] { return "pending state"; })
          .when(
              bind(lit(Status::Running)) >>
              [](int whole) {
                std::cout << "Captured (as int): " << whole << "\n";
                return "running state";
              })
          .when(__ >> [] { return "other state"; })  // Wildcard pattern
          .otherwise([] { return "otherwise"; });

  std::cout << "Result = " << result << "\n";
  
  // Example with wildcard as fallback
  int value = 42;
  std::cout << match(value)
      .when(lit(0) >> []{ std::cout << "zero"; })
      .when(lit(1) >> []{ std::cout << "one"; })
      .when(__ >> []{ std::cout << "other number"; })  // Matches any other value
      .end();
}
```

**Output:**
```
Captured (as int): 1
Result = running state
other number
```

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
- `.otherwise(handler)` - Default fallback case with explicit handler
- `.end()` - Terminate exhaustive void-only matches

**Terminal Methods:**
- Use `.otherwise(handler)` when you need default behavior or non-void returns
- Use `.end()` for exhaustive matching where all cases return void

### Pattern Types
- `lit(value)` - Exact value matching for any comparable type
- `lit_ci(value)` - Case-insensitive ASCII string matching
- `__` - Wildcard pattern that matches any value without binding
- `bind()` - Capture the entire subject value
- `bind(subpattern)` - Capture subject after matching with subpattern

### Supported Value Types
- Arithmetic types (int, double, float, etc.)
- Enum types
- String types (std::string, std::string_view, const char*)
- User-defined types (must support operator==)

### Handler Types
- Value handlers (return fixed value)
- Function handlers (process matched value)
- Capturing handlers (receive bound values)

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
   Follow **[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html#Comment_Style)** for documentation comments.
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

## 🔮 Future Roadmap

### Exhaustiveness Checking (Planned)

Patternia plans to introduce **compile-time exhaustiveness checking** to provide enhanced safety guarantees:

**Planned Features:**
- **Enum Coverage Detection** - Catch missing enum variants at compile-time  
- **Wildcard Analysis** - Proper handling of `__` patterns in exhaustiveness
- **Enhanced Error Messages** - Clear diagnostics for missing cases

**Example (Future API):**
```cpp
enum class Status { Pending, Running, Completed, Failed };

// This will trigger a compile-time error for missing variants
match(status)
    .when(lit(Status::Running) >> "running")
    .when(lit(Status::Completed) >> "completed")
    .end();  // ❌ ERROR: Not all enum variants covered!
```

This future feature will make Patternia even safer by preventing runtime bugs through compile-time guarantees.

---

<div align="center">
  <sub>Built with ❤️ for modern C++ development</sub>
</div>
