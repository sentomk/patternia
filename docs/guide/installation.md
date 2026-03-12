# Installation

Patternia is a header-only C++ library.
There is no build system, no linking step, and no external dependency.

You can integrate Patternia into your project in any of the following ways.

---

## 1. vcpkg

```bash
vcpkg install patternia
```

Or via `vcpkg.json` manifest:

```json
{
  "dependencies": ["patternia"]
}
```

Then in your CMake project:

```cmake
find_package(patternia CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE patternia::patternia)
```

Pass the vcpkg toolchain file to CMake if it is not already in your preset:

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

---

## 2. Fetch Patternia automatically

You may choose to fetch Patternia as a remote dependency during configuration.

**FetchContent:**

```cmake
include(FetchContent)
FetchContent_Declare(patternia
  GIT_REPOSITORY https://github.com/sentomk/patternia.git
  GIT_TAG main
)
FetchContent_MakeAvailable(patternia)

target_link_libraries(your_target PRIVATE patternia::patternia)
```

**CPM.cmake:**

```cmake
CPMAddPackage("gh:sentomk/patternia@main")
```

This fits well in reproducible CI pipelines and modern CMake workflows.

---

## 3. Install and consume via `find_package()`

Patternia provides full CMake package export support.

Install:

```bash
cmake -B build
cmake --build build
cmake --install build --prefix /path/to/install
```

Use in any project:

```cmake
find_package(patternia REQUIRED)
target_link_libraries(your_target PRIVATE patternia::patternia)
```

This is the recommended method for system-wide integration, packaging, and distribution.

---

## 4. Add Patternia as a Git submodule

```bash
git submodule add https://github.com/sentomk/patternia extern/patternia
```

Then in your CMake project:

```cmake
add_subdirectory(extern/patternia)
target_link_libraries(your_target PRIVATE patternia::patternia)
```

This approach keeps Patternia version-controlled inside your repository and works well for mid- to large-scale projects.

---

## 5. Include the `include/` directory directly

This is the simplest approach.
Just add Patternia’s header path to your target:

```cmake
target_include_directories(your_target
  PRIVATE /path/to/patternia/include
)
```

No configuration, no installation, immediately usable.