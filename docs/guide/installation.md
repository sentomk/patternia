# Installation

Patternia is a header-only C++ library.  
There is no build system, no linking step, and no external dependency.

You can integrate Patternia into your project in any of the following ways.

---

## Using CMake (Recommended)

Clone the repository and add it as a subdirectory:

```cmake
add_subdirectory(external/patternia)

target_link_libraries(your_target PRIVATE patternia)
