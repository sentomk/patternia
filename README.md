## patternia

A header-only, zero-overhead, compile-time pattern matching library for modern C++.

### Table of Contents

- [Features](#features)
- [Examples](#examples)
- [Installation](#installation)
- [Roadmap](#patternia-technology-roadmap)

### Features

- **Header-only**  
  No source files to compile—simply include the headers and you’re ready to go.
- **Compile-time pattern matching**  
  Entirely inlined and constexpr-friendly; no runtime closure or lambda overhead.
- **Rich built-in patterns**
  - _Value patterns_ (`value(v)`, `ci_value(v)`) for equality (and case-insensitive)
  - _Relational patterns_ (`lt(v)`, `le(v)`, `gt(v)`, `ge(v)`, `eq(v)`, `ne(v)`) for ordering comparisons
  - _Range patterns_ (`between(lo, hi, closed)`) for closed or open intervals
- **Fluent DSL**  
  Chain `.when(p >> handler)` calls for concise, readable match expressions, with automatic overload resolution for
  handlers that take zero or one argument.
- **Lightweight dependency**  
  Depends only on the C++ standard library; no external libraries required unless you opt in to benchmarks or tests.

### Examples

- #### value pattern

  ```cpp
  int x = 42;
  auto parity = match(x)
      .when(value(0) >> "zero")
      .when(value(1) >> "one")
      .when(value(2) >> "two")
      .otherwise(x % 2 == 0 ? "even" : "odd");
  std::cout << "x is " << parity << "\n";
  ```

  ```cpp
  int score = 85;
  auto grade = match(score)
      .when(lt(60) >> 'F')
      .when(between(60, 70, false) >> 'D')
      .when(between(70, 80, false) >> 'C')
      .when(between(80, 90, false) >> 'B')
      .otherwise('A');
  std::cout << "grade: " << grade << "\n";
  ```

  - #### relational pattern

  ```cpp
  double n = -3.14;
  auto sign = match(n)
      .when(lt(0.0)     >> "negative")
      .when(eq(0.0)     >> "zero")
      .when(gt(0.0)     >> "positive")
      .otherwise("NaN");
  std::cout << "n is " << sign << "\n";
  ```

  ```cpp
  int age = 30;
  auto category = match(age)
      .when(lt(18)      >> "minor")
      .when(between(18, 65, false) >> "adult")
      .when(ge(65)      >> "senior")
      .otherwise("unknown");
  std::cout << "age group: " << category << "\n";
  ```

  - #### case expression DSL

  ```cpp
  std::string cmd = "StArT";
  match(cmd)
      .when(ci_value("start") >> []{ std::cout << "starting...\n"; })
      .when(ci_value("stop")  >> []{ std::cout << "stopping...\n"; })
      .otherwise([]{ std::cout << "unknown command\n"; });
  ```

  ```cpp
  char c = 'x';
  auto type = match(c)
      .when(value('a') >> "vowel")
      .when(value('e') >> "vowel")
      .when(value('i') >> "vowel")
      .when(value('o') >> "vowel")
      .when(value('u') >> "vowel")
      .otherwise("consonant");
  std::cout << c << " is a " << type << "\n";
  ```

  - #### `.with()` / `.otherwise()` style

  ```cpp
  auto result = match(7)
      .with(value(3), []{ return "three"; })
      .with(lt(5),    []{ return "small"; })
      .with(gt(10),   []{ return "large"; })
      .otherwise([]{ return "medium"; });
  std::cout << result << "\n";
  ```

  ```cpp
  struct Point { int x, y; };
  Point p{0, 0};
  auto quadrant = match(p)
    .with([](auto& pt){ return pt.x > 0 && pt.y > 0; }, "I")
    .with([](auto& pt){ return pt.x < 0 && pt.y > 0; }, "II")
    .with([](auto& pt){ return pt.x < 0 && pt.y < 0; }, "III")
    .with([](auto& pt){ return pt.x > 0 && pt.y < 0; }, "IV")
    .otherwise("origin");
  std::cout << "Quadrant: " << quadrant << "\n";

  ```

### Installation

#### To build and use

Ensure your development environment has the following tools installed and configured:

- [CMake (≥3.14)](https://cmake.org/)
- **Minimum**: C++17
- **Recommended compilers**: GCC >= 11, Clang >= 12, MSVC >= Visual Studio 2019

#### Option A: FetchContent (recommended)

Add the following to your **project’s top-level `CMakeLists.txt`**:

```cmake
cmake_minimum_required(VERSION 3.14)
project(your_project_name LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        patternia
        GIT_REPOSITORY https://github.com/SentoMK/patternia.git
        GIT_TAG v0.3.2
)
FetchContent_MakeAvailable(patternia)

add_executable(your_project_name main.cpp)
target_link_libraries(your_project_name PRIVATE patternia::patternia)
```

#### Option B: Build & Install from Source

Clone the repository and build:

```bash
git clone https://github.com/SentoMK/patternia.git
cd patternia
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix ./install
```

This will install headers and CMake config files into `./install/include` and `./install/lib/cmake/patternia`

You can then use it in your own project:

```cmake
find_package(patternia REQUIRED PATHS /absolute/path/to/patternia/install)
target_link_libraries(your_project_name PRIVATE patternia::patternia)
```

#### Optional CMake Flags

- `-DBUILD_BENCHMARKS=ON`

  Build benchmark executables (requires [Google Benchmark](https://github.com/google/benchmark), fetched automatically).

  - Recommended to also set `-DHAVE_STD_REGEX=ON` for benchmark builds.

- `-DBUILD_TESTS=ON`

  Build unit tests (requires [GoogleTest](https://github.com/google/googletest), fetched automatically).

- `-DPTN_SKIP_COMPILER_CHECK=ON`

  Skip compiler version checks (use with caution, for non-standard toolchains or CI environments).

**Example:**

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=ON \
  -DBUILD_BENCHMARKS=ON \
  -DHAVE_STD_REGEX=ON
cmake --build build
```

### Patternia Technology Roadmap

<br>

|             **Phase**              | **Version** |                  **Goal**                  |    **Status**     |
| :--------------------------------: | :---------: | :----------------------------------------: | :---------------: |
|       **P0-Core Foundation**       |  `v0.3.x`   |        Minimal DSL and core engine         |      ✅ Done      |
|  **P1 – Predicate & Type Layer**   |  `v0.4.x`   |    Extend to predicates and type checks    |  🚧 In progress   |
| **P2 – Logical & Wildcard Layer**  |  `v0.5.x`   |     Add logical composition & wildcard     |  🚧 In progress   |
|    **P3 – Structured Matching**    |  `v0.6.x`   |         Tuple and guarded matching         |    🧩 Planned     |
| **P4 – Compile-Time Optimization** |  `v0.7.x`   |    Constexpr dispatch & concept checks     |    🧩 Planned     |
|   **P5 – Expression DSL Fusion**   |  `v0.8.x`   |     Operator-based pattern composition     |    🧩 Planned     |
|     **P6 – Reflection Layer**      |  `v0.9.x`   | Structural pattern matching via reflection |  🔬 Experimental  |
|     **P7 – Runtime Dispatch**      |  `v1.0.0`   |      Fast runtime jump-table dispatch      | 🚀 Target Release |
