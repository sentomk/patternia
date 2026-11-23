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
  - `release/X.Y.Z` for a release (no `v` prefix)
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
- After changing anything under `include/` (or the version), regenerate
  the single header with `python3 scripts/amalgamate.py` and commit
  `single_include/ptn/patternia.hpp`; CI fails when it is out of date.
  `-DPTN_TEST_SINGLE_HEADER=ON` also runs the tests against it.

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

Do not add attribution trailers (`Co-authored-by:`, AI "Generated
with" lines) to commits, pull requests, release notes or downstream
pull requests such as vcpkg port updates.

---

## Versioning

Patternia follows [Semantic Versioning](https://semver.org/). Before
1.0 the minor number plays the role of the major one, as in Cargo: a
release that can break code or change what it does bumps the minor.

| Release contains | Version |
|---|---|
| New public API, removed or renamed API, or a change in what existing code does at run time or compile time | next minor: `0.MINOR.0` |
| Only bug fixes, packaging or install fixes, documentation, or performance work with no observable behavior change | next patch: `0.MINOR.PATCH` |

- A patch release never adds public API. If a fix needs new API, it
  waits for the next minor release.
- One new feature is enough to make a release minor. Do not ship
  features in a patch because the release is small.
- Remove API in two steps: deprecate it in one minor release (with a
  replacement and a migration note) and remove it in a later one.
- From 1.0 on, the usual rules apply: breaking changes bump the major
  version, new API bumps the minor, fixes bump the patch.
- The roadmap names planned releases by minor version (`v0.10`,
  `v0.11`); the first release of a series is `X.Y.0`.

Releases up to v0.9.7 predate this policy: several 0.9.x patch releases
added features and v0.9.4 removed API. The policy applies from v0.10.0.

---

## Development Setup

```bash
git clone https://github.com/SentoMK/patternia.git
cd patternia
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Pinned formatter

Formatting is checked with the exact clang-format version listed in
`tools/requirements.txt`, the same one CI uses. `tools/envsetup.py`
installs it into the ignored `.tools/venv` directory on first use; you
only need Python 3 with `venv` support (Debian/Ubuntu: `python3-venv`).

```bash
python3 scripts/format.py          # check files changed against origin/main
python3 scripts/format.py --fix    # rewrite them in place
python3 scripts/format.py --all    # check every tracked C/C++ file
python3 tools/envsetup.py --versions   # install and print the pinned version
```

Formatting is only enforced for files a pull request touches, so
`--all` may list older files that have not been reformatted yet.

Optional: enable the local pre-commit hook, which runs
`scripts/format.py --fix --staged` (recommended):

```bash
bash scripts/setup-git-hooks.sh
```

On Windows PowerShell:

```powershell
pwsh -File .\scripts\setup-git-hooks.ps1
```

The setup scripts set `core.hooksPath` for this repository only. If you
already use a global `core.hooksPath` (for example company-wide hooks),
that setting is replaced for this repository, so skip the setup script
and run `scripts/format.py --fix` yourself instead.

The pinned versions of the other CI dependencies live next to it:
`tools/requirements-bench.txt` (benchmark charts) and
`tools/requirements-docs.txt` (documentation site).
