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

## 2. Conan

The Conan 2 recipe lives in `packaging/conan/` and is being submitted
to ConanCenter. Until it is available there, export it into your local
Conan cache from a checkout of this repository:

```bash
conan create packaging/conan/all --version 0.9.7
```

Then require it from your `conanfile.txt`:

```ini
[requires]
patternia/0.9.7

[generators]
CMakeDeps
CMakeToolchain

[layout]
cmake_layout
```

The package provides the same CMake target as the other methods:

```cmake
find_package(patternia CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE patternia::patternia)
```

Install the dependencies and build through the presets that
`CMakeToolchain` generates:

```bash
conan install . -s compiler.cppstd=17
cmake --preset conan-release
cmake --build --preset conan-release
```

Patternia needs C++17. Pass `-s compiler.cppstd=17` (or newer), or set
`compiler.cppstd` in your Conan profile, when the compiler defaults to
an older standard.

---

## 3. Fetch Patternia automatically

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

## 4. Install and consume via `find_package()`

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

## 5. Add Patternia as a Git submodule

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

## 6. Include the `include/` directory directly

This is the simplest approach.
Just add Patternia’s header path to your target:

```cmake
target_include_directories(your_target
  PRIVATE /path/to/patternia/include
)
```

No configuration, no installation, immediately usable.