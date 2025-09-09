## patternia

A header-only, zero-overhead, compile-time pattern matching library for modern C++.

### Table of Contents

- [Features]()
- [Benchmarks]()
- [Examples]()
- [Installation](#installation)
- [Contributing]()

### Installation

#### To build and use

Ensure your development environment has the following tools installed and configured:

- [CMake (≥3.21)](https://cmake.org/)
- A C++20-capable compiler toolchain of one of the combinations below:
  - **Visual Studio on Windows**
  
    - Generator: [Visual Studio 17 2022](https://visualstudio.microsoft.com/vs/)
    - Compiler/stdlib: [MSVC’s `cl.exe` + MSVC STL](https://visualstudio.microsoft.com/vs/) 
  
  - **Clang 18.x + MinGW-w64/libstdc++ (native Windows)**
    
    - Generator: [ninja](https://github.com/ninja-build/ninja)
    - Compiler: [MSYS2 MinGW-w64’s clang++ 18.x](https://www.msys2.org/)
    - Standard library: [GNU libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/)
  
  - **Clang + libstdc++ (WSL2 or Linux)**
    
    - Generator: [ninja](https://github.com/ninja-build/ninja)
    - Compiler: **system** `clang++`
    - Standard library: **system** `libstdc++`
  
  - **Clang + libc++ (WSL2 or Linux)**

    - Generator: [ninja](https://github.com/ninja-build/ninja)
    - Compiler: **system** `clang++` with `-stdlib=libc++`
    - Standard library: [LLVM libc++](https://libcxx.llvm.org/)

  - **GCC + libstdc++ (WSL2 or Linux)**

    - Generator: [ninja](https://github.com/ninja-build/ninja) or [Unix Makefiles](https://cmake.org/cmake/help/latest/generator/Unix%20Makefiles.html)
    - Compiler:  `g++`

  - **Xcode on macOS**

    - Generator: [ninja](https://github.com/ninja-build/ninja) or [Unix Makefiles](https://cmake.org/cmake/help/latest/generator/Unix%20Makefiles.html)
    - Compiler: **system** `clang++` (Apple Clang via Xcode Command-Line Tools)
    - Standard library: Apple’s `libc++` (bundled with Xcode CLT)