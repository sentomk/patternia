# Contributing to Patternia

Contributions are welcome. Whether it is **bug reports**, **feature proposals**, or **pull requests**, your help is appreciated.

Please note that this project is governed by a Code of Conduct.  
By participating, you are expected to uphold it.

See [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) for details.

---

## Before Submitting Changes

### 1. Branch and Pull Request Workflow

The `main` branch is protected and should remain releasable.

- Do not push directly to `main`
- Create focused branches for changes:
  - `feat/<topic>` for new features
  - `fix/<topic>` for bug fixes
  - `docs/<topic>` for documentation
  - `test/<topic>` for test-only changes
  - `chore/<topic>` for maintenance
- Open a pull request into `main`
- Keep each pull request focused on one goal
- Ensure required CI passes before merging

For concurrent local work, prefer `git worktree` over repeatedly stashing and
switching branches:

```bash
git worktree add ../patternia-feat-example -b feat/example
git worktree add ../patternia-fix-example -b fix/example
git worktree list
git worktree remove ../patternia-feat-example
```

Pull request titles should follow the same style as commit subjects, such as
`feat(pattern): add case-insensitive string matching` or
`fix(eval): correct match_result type inference`.

Pull requests are validated automatically by CI
(`.github/workflows/validate.yml`). Requirements:

- **PR title** must follow the conventional commit format:
  `type(scope): description`.
- **PR body** must be at least 50 characters and contain three
  sections, each as a bold header on its own line:
  - `**Summary**` — overview of the change
  - `**Changes**` — bullet list of what changed
  - `**Testing**` — how the change was verified
- **Exactly one commit** per pull request. Pull requests are
  squash-merged, so a single-commit branch keeps the result
  predictable:

  ```bash
  git reset --soft $(git merge-base HEAD origin/main)
  git commit -m 'type(scope): description'
  git push --force-with-lease
  ```

### 2. Discuss First

For non-trivial changes (new features, API changes, or behavior modifications),  
please open an **Issue** or **Discussion** before submitting a pull request.

This helps ensure alignment with the project’s design goals.

### 3. Code Style and Design Principles

Patternia is a design-focused C++ library. Please ensure that contributions adhere to the following principles:

- **C++17 or later**
- **No RTTI or virtual dispatch**
- **Zero-overhead abstractions**
- Maintain consistency with existing DSL and API design

### 4. Tests Required

All logic changes must be accompanied by appropriate tests.

- Add new tests under `tests/`
- Update existing tests when modifying behavior
- Ensure all tests pass before submitting

### 5. Comment Style

Follow the  
**[Google C++ Style Guide – Comment Style](https://google.github.io/styleguide/cppguide.html#Comment_Style)**  
for documentation comments.

### 6. Commit Style

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

The subject line is checked mechanically by CI
(`scripts/check_commit_msg.py`):

- Description uses the imperative mood ("add", not "added"/"adds").
- Description starts lowercase and has no trailing period.
- Description is specific, not a vague placeholder
  ("fix bug", "wip", "update").
- Subjects over 72 characters trigger a warning (aim for 50-72).

Prose lines in the body should wrap at 72 characters; CI enforces
this as well (bullet and indented lines are exempt).

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
