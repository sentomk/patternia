# Contributing to Patternia

Contributions are welcome. Whether it is **bug reports**, **feature proposals**, or **pull requests**, your help is appreciated.

Please note that this project is governed by a Code of Conduct.  
By participating, you are expected to uphold it.

See [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) for details.

---

## Before Submitting Changes

### 1. Discuss First

For non-trivial changes (new features, API changes, or behavior modifications),  
please open an **Issue** or **Discussion** before submitting a pull request.

This helps ensure alignment with the project’s design goals.

### 2. Code Style and Design Principles

Patternia is a design-focused C++ library. Please ensure that contributions adhere to the following principles:

- **C++17 or later**
- **No RTTI or virtual dispatch**
- **Zero-overhead abstractions**
- Maintain consistency with existing DSL and API design

### 3. Tests Required

All logic changes must be accompanied by appropriate tests.

- Add new tests under `tests/`
- Update existing tests when modifying behavior
- Ensure all tests pass before submitting

### 4. Comment Style

Follow the  
**[Google C++ Style Guide – Comment Style](https://google.github.io/styleguide/cppguide.html#Comment_Style)**  
for documentation comments.

### 5. Commit Style

Use clear, conventional commit messages, for example:

```

feat: add case-insensitive string matching
fix: correct match_result type inference
refactor: reorganize DSL operators

````

---

## Development Setup

```bash
git clone https://github.com/SentoMK/patternia.git
cd patternia
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
````