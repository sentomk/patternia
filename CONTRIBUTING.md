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

Use clear, conventional commit messages with a structured body.

Recommended format:

- **Subject line**: `type(scope): short summary`
- Use imperative mood and keep the subject concise (around 50-72 chars)
- Add a body for non-trivial changes, grouped by purpose
- Use bullet lists in body sections; do not encode line breaks as literal `\n`

Suggested body sections:

- `Problem`
- `Implementation`
- `Tests`
- `Notes`

Example:

```
perf(eval): avoid duplicate binding in guarded match path

Problem
- Guarded cases could bind twice on a hit path.
- This added avoidable overhead on heavy-bind workloads.

Implementation
- Added a guarded fast path in typed evaluation.
- Bound once and reused the same tuple for guard + handler.

Tests
- Added bind-count regression tests for guarded and fallthrough paths.

Notes
- Internal optimization only.
- No DSL/API surface changes.
```

Additional valid one-line examples:

```
feat(pattern): add case-insensitive string matching
fix(eval): correct match_result type inference
refactor(dsl): reorganize case expression operators
```

---

## Development Setup

```bash
git clone https://github.com/SentoMK/patternia.git
cd patternia
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Optional: enable local pre-commit formatting hook (recommended):

```bash
bash scripts/setup-git-hooks.sh
```

On Windows PowerShell:

```powershell
pwsh -File .\scripts\setup-git-hooks.ps1
```
